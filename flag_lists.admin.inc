<?php
// $Id$

/**
 * @file
 * Contains administrative pages for creating, editing, and deleting flag lists.
 */

/**
 * Flag administration page. Display a list of existing flags.
 */
function flag_lists_admin_page() {
  return theme('flag_lists_admin_page');
}

/**
 * Theme the output for the main flag administration page.
 */
function theme_flag_lists_admin_page() {
  drupal_set_title(t('Flag lists'));
  $output = '<p>' . t('This page shows all the <em>lists</em> that are currently defined on this system.') . '</p>';

  // Can we use our default view?
  if (module_exists('views')) {
    views_include('view');
    $view = view::load('flag_lists', FALSE);
    $default_view = views_get_default_view('flag_lists');
    if (!empty($view) || !empty($default_view)) {
      $output .= views_embed_view('flag_lists');
    }
  }

  // Else build a simplified display.
  else {
    $header = array(
      array('data' => t('List'), 'field' => 'name'),
      array('data' => t('List title'), 'field' => 'title'),
      array('data' => t('List type')),
      array('data' => t('Owner'), 'field' => 'uid'),
      array('data' => t('Node types')),
      array('data' => t('Operations')),
    );
    $flags = flag_lists_get_flags(25, $header);
    foreach ($flags as $flag) {
      $ops = theme('flag_lists_ops', $flag);

      $name = db_result(db_query("SELECT name from {users} WHERE uid = %d", $flag->uid));
      $user = l($name, 'user/'. $flag->uid, array('attributes' => array('title' => t('View user profile.'))));
      $rows[] = array(
        $flag->name,
        $flag->title,
        $flag->content_type,
        $user,
        $flag->types ? implode(', ', $flag->types) : '-',
        $ops,
      );
    }
    if (!$flags) {
      $rows[] = array(
        array('data' => t('No flags are currently defined.'), 'colspan' => 6),
      );
    }

    $output .= theme('table', $header, $rows);
    $output .= theme('pager', NULL, 25, 0);
  }

  return $output;
}

function flag_lists_add($type = NULL) {
  if (is_null($type)) {
    drupal_access_denied();
  }
  
  return drupal_get_form('flag_lists_form', NULL, $type);
}

/**
 * Form to Add or edit a list.
 */
function flag_lists_form(&$form_state, $name = NULL, $type = NULL) {
  // First some sanity checks. $name and $type can't both be NULL.
  // There must be a template for this content type.
  if (is_null($name) && is_null($type)) {
    drupal_access_denied();
  }
  if (!flag_lists_template_exists($type) && is_null($name)) {
    return;
  }
  // If name is numeric, then we have the fid, so get the name.
  if (is_numeric($name)) {
    $name = db_result(db_query("SELECT name from {flag_lists_flags} WHERE fid = %s", $name));
  }
  // Adding a new list.
  if (!isset($name)) {
    drupal_set_title(t('Add a new list'));
    $form['edit'] = array(
      '#type' => 'hidden',
      '#value' => FALSE,
    );
  }

  // Editing an existing list.
  else {
    $flag = flag_lists_get_flag($name);
    drupal_set_title(t('Edit your "@title" @name title', array('@title' => $flag->get_title(), '@name' => variable_get('flag_lists_name', t('list')))));
    $form['edit'] = array(
      '#type' => 'hidden',
      '#value' => TRUE,
    );
    $form['name'] = array(
      '#type' => 'hidden',
      '#value' => $name,
    );
  }
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $flag->title,
    '#description' => t('A short, descriptive title for this @name list. Limit to 255 characters.', array('@name' => variable_get('flag_lists_name', t('list')))),
    '#maxlength' => 255,
    '#required' => TRUE,
    '#access' => empty($flag->locked['title']),
    '#weight' => -2,
  );
  $form['type'] = array(
    '#type' => 'hidden',
    '#value' => $type,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Validate a list.
 */
function flag_lists_form_validate($form, $form_state) {
  // Ensure 255 charactor or less name.
  if (drupal_strlen($form_state['values']['title']) > 255) {
    form_set_error('name', t('The @name title may only be 32 characters long.', array('@name' => variable_get('flag_lists_name', t('list')))));
  }
  // Ensure the machine name is unique.
  if (!$form_state['values']['edit']) {
    if (flag_lists_title_exists($form_state['values']['title'], $form_state['values']['type'])) {
      form_set_error('title', t('You already have a @name with this name for this type of content.', array('@name' => variable_get('flag_lists_name', t('list')))));
    }
  }
}

/**
 * Save a list.
 */
function flag_lists_form_submit($form, $form_state) {
  if ($form_state['values']['edit']) {
    // Editing the title.
    $flag = flag_lists_get_flag($form_state['values']['name']);
    $flag->title = $form_state['values']['title'];
    flag_lists_set_messages($flag);
    flag_lists_save($flag);
    _flag_lists_clear_cache();
  }
  else {
    // New flag.
    global $user;

    // Load the template row.
    $type = $form_state['values']['type'];
    $row = db_fetch_object(db_query("SELECT * from {flags} f LEFT JOIN {flag_lists_types} fl on f.name = fl.name WHERE fl.type = '%s'", $type));

    $newflag = flag_flag::factory_by_content_type('node');

    $flag = $newflag->factory_by_row($row);
    // The template fid becomes the flag_lists parent flag.
    $flag->pfid = $row->fid;
    // The fid is NULL because this is really a new flag.
    $flag->fid = NULL;
    // The name is created in the save function.
    $flag->name = NULL;
    $flag->link_type = 'toggle';
    $flag->title = $form_state['values']['title'];;
    $flag->types = array($type);
    $flag->uid = $user->uid;
    flag_lists_set_messages($flag);
    // Save it.
    flag_lists_save($flag);
  }
}

/**
 * Flag lists settings page.
 */
function flag_lists_settings_form(&$form_state) {
  drupal_set_title('Flag lists settings');
  $form['text'] = array(
    '#title' => t('Using flag lists'),
    '#value' => t('Flag lists allow users to create their own personal flags.
      No user can add to another user\'s lists. Lists inherit their
      settings from template flags, which exist as flags in the flags\' module.'
    ),
  );

  $form['flag_lists_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Substitute "list" with your own terminology'),
    '#default_value' => variable_get('flag_lists_name', t('list')),
    '#description' => t('You can choose to use another name for "list", such as "favorites" or "bookmarks". Lowercase and plural names usually work best.'),
    '#required' => TRUE,
  );
  $form['rebuild'] = array(
    '#type' => 'fieldset',
    '#title' => t('Global rebuild'),
    '#description' => t('Changes to list templates and  settings normally apply
      to only newly created flags. However, you can globally apply changes here.
      First adjust the settings above and the list templates and save them. Then
      click the link below. This will change ALL existing flag lists. It cannot
      be undone.'),
    '#tree' => FALSE,
  );
  $form['rebuild']['global_rebuild'] = array(
    '#value' => l('Rebuild all flag lists.', 'admin/build/flag_lists/rebuild'),
  );

  return system_settings_form($form);
}

/**
 * Confirm global settings rebuild.
 */
function flag_lists_rebuild_confirm($form_state) {
  return confirm_form($form,
    t('Are you sure you want to rebuild all flag lists?'),
    'admin/build/flags/lists/settings',
    t('This action cannot be undone.'),
    t('Rebuild'), t('Cancel')
  );
}

/**
 * Confirm global settings rebuild.
 */
function flag_lists_rebuild_confirm_submit($form, $form_state) {
  flag_lists_rebuild();
  drupal_set_message(t('All flag lists have been rebuilt.'));
  drupal_goto('admin/build/flags/lists/settings');
}

/**
 * Delete flag lists page.
 */
function flag_lists_delete_confirm(&$form_state, $name) {
  $flag = flag_lists_get_flag($name);
  if (empty($flag)) {
    drupal_goto();
  }

  $form['fid'] = array('#type' => 'value', '#value' => $flag->fid);

  return confirm_form($form,
    t('Are you sure you want to delete %title?', array('%title' => $flag->get_title())),
    isset($_GET['destination']) ? $_GET['destination'] : '/',
    t('This action cannot be undone.'),
    t('Delete'), t('Cancel')
  );
}

function flag_lists_delete_confirm_submit($form, &$form_state) {
  $flag = flag_lists_get_flag($form_state['values']['fid']);
  if ($form_state['values']['confirm']) {
    flag_lists_fl_delete($flag);
    _flag_lists_clear_cache();
  }
}

/**
 * Form to create a new template.
 */
function flag_lists_create_template_form(&$form_state) {
  drupal_set_title(t('Add a new list template'));
  $form['help'] = array(
    '#value' => t('This form creates a new, blank list template. After saving, you will be able to configure further options.'),
  );
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Template name'),
    '#description' => t('The machine-name for this template. It may be up to 32 characters long and my only contain lowercase letters, underscores, and numbers.'),
    '#maxlength' => 255,
    '#required' => TRUE,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * New template validation.
 */
function flag_lists_create_template_form_validate($form, &$form_state) {
  // Ensure a safe machine name.
  if (!preg_match('/^[a-z_][a-z0-9_]*$/', $form_state['values']['name'])) {
    form_set_error('name', t('The flag name may only contain lowercase letters, underscores, and numbers.'));
  }
  // Ensure 32 charactor or less name.
  if (drupal_strlen($form_state['values']['name']) > 32) {
    form_set_error('name', t('The flag name may only be 32 characters long.'));
  }
  // Ensure the machine name is unique.
  $flag = flag_get_flag($form_state['values']['name']);
  if ($flag) {
    form_set_error('name', t('Flag names must be unique. This flag name is already in use.'));
  }
}

/**
 * New template submit.
 */
function flag_lists_create_template_form_submit($form, &$form_state) {
  $template = flag_lists_flag_default_flags('fl_template_'. $form_state['values']['name']);
  $flag = flag_flag::factory_by_array($template[0]);
  $flag->title = t('List template '). $form_state['values']['name'];
  $flag->save();
  // Enter the new template into flag_lists_types.
  db_query("INSERT INTO {flag_lists_types} (name) VALUES ('%s')", $flag->name);
  $form_state['redirect'] = 'admin/build/flags/edit/'. $flag->name;
}