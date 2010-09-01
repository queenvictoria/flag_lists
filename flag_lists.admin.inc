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
  $flags = flag_lists_get_flags();
  return theme('flag_lists_admin_page', $flags);
}

/**
 * Theme the output for the main flag administration page.
 */
function theme_flag_lists_admin_page($flags) {
  drupal_set_title(t('Flag lists'));
  $output = '<p>' . t('This page lists all the <em>flag lists</em> that are currently defined on this system.') . '</p>';

  // Build the list of normal, database flags.
  foreach ($flags as $flag) {
    $ops = theme('links', array(
      'flags_edit' =>  array('title' => t('edit'), 'href' => 'admin/build/flags/edit/' . $flag->name, 'query' => drupal_get_destination()),
      'flags_delete' =>  array('title' => t('delete'), 'href' => 'admin/build/flags/delete/' . $flag->name, 'query' => drupal_get_destination()),
    ));
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

  $header = array(t('Flag'), t('Flag title'), t('Flag type'), t('Owner'), t('Node types'), t('Operations'));
  $output .= theme('table', $header, $rows);

  return $output;
}


/**
 * Add/Edit flag lists page.
 */
function flag_lists_form(&$form_state, $type, $name = NULL) {
  if (!isset($name)) {
    // Adding a new flag.
    drupal_set_title(t('Add new @name', array('@name' => variable_get('flag_lists_name', t('Flag')))));
  }
  else {
    // Editing an existing flag.
    $flag = flag_lists_get_flag($type);
    drupal_set_title(t('Edit your "@title" @name title', array('@title' => $flag->get_title(), '@name' => variable_get('flag_lists_name', t('Flag')))));
    $form['edit'] = array(
      '#type' => 'hidden',
      '#value' => TRUE,
    );
  }

  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $flag->title,
    '#description' => t('A short, descriptive title for this @name.', array('@name' => variable_get('flag_lists_name', t('Flag')))),
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
 * Flag lists settings page.
 */
function flag_lists_settings_form(&$form_state) {
  drupal_set_title('Flag lists settings');
  $form['text'] = array(
    '#title' => t('Using flag lists'),
    '#value' => t('Flag lists allow users to create their own personal flags.
      No user can see (unless in a public view from the views module) or set
      another user\'s personal flags. These settings apply globally to all flag
      lists created by all users. Changing a setting here will only affect newly
      created flag lists.'),
  );

  $form['flag_lists_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Substitute "Flag" with your own terminology'),
    '#default_value' => variable_get('flag_lists_name', t('flags')),
    '#description' => t('You can choose to use another name for "Flags", such as "List" or "Bookmarks".'),
    '#required' => TRUE,
  );

  $form['messages'] = array(
    '#type' => 'fieldset',
    '#title' => t('Messages'),
  );

  $form['messages']['flag_lists_short'] = array(
    '#type' => 'textfield',
    '#title' => t('Flag lists link prefix text'),
    '#default_value' => variable_get('flag_lists_short', t('Add this to your')),
    '#description' => t('The prefix text for the "flag this" link for this flag.'),
    '#required' => TRUE,
  );

  $form['messages']['flag_lists_long'] = array(
    '#type' => 'textfield',
    '#title' => t('Flag lists link description prefix'),
    '#default_value' => variable_get('flag_lists_long', t('Click here to add this to your')),
    '#description' => t('The description of the "flag this" link. Usually displayed on mouseover.'),
  );

  $form['messages']['flag_lists_message'] = array(
    '#type' => 'textfield',
    '#title' => t('Flagged message prefix'),
    '#default_value' => variable_get('flag_lists_message', t('This has been added to your')),
    '#description' => t('Message displayed after flagging content. If JavaScript is enabled, it will be displayed below the link. If not, it will be displayed in the message area.'),
  );

  $form['messages']['unflag_lists_short'] = array(
    '#type' => 'textfield',
    '#title' => t('Unflag link prefix text'),
    '#default_value' => variable_get('unflag_lists_short', t('Remove this from your')),
    '#description' => t('The text for the "unflag this" link prefix for this flag.'),
    '#required' => TRUE,
  );

  $form['messages']['unflag_lists_long'] = array(
    '#type' => 'textfield',
    '#title' => t('Unflag link description prefix'),
    '#default_value' => variable_get('unflag_lists_long', t('Click here to remove this from your')),
    '#description' => t('The description of the "unflag this" link. Usually displayed on mouseover.'),
  );

  $form['messages']['unflag_lists_message'] = array(
    '#type' => 'textfield',
    '#title' => t('Unflagged message prefix'),
    '#default_value' => variable_get('unflag_lists_message', t('This has been removed from your')),
    '#description' => t('Message displayed after content has been unflagged. If JavaScript is enabled, it will be displayed below the link. If not, it will be displayed in the message area.'),
  );
  $form['display'] = array(
    '#type' => 'fieldset',
    '#title' => t('Display options'),
    '#description' => t('Flags are usually controlled through links that allow users to toggle their behavior. You can choose how users interact with flags by changing options here. It is legitimate to have none of the following checkboxes ticked, if, for some reason, you wish <a href="@placement-url">to place the the links on the page yourself</a>.', array('@placement-url' => 'http://drupal.org/node/295383')),
    '#tree' => FALSE,
  );
  $form['display']['flag_lists_use_links'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display flag lists in links'),
    '#description' => t('Unchecking this will prevent flag lists from appearing in links. You will need to display them using the provided block or the flag api.'),
    '#default_value' => variable_get('flag_lists_use_links', 1),
  );
  $form['display']['flag_lists_link_type'] = array(
    '#type' => 'radios',
    '#title' => t('Link type'),
    '#options' => _flag_link_type_options(),
    '#option_descriptions' => _flag_link_type_descriptions(),
    '#flag_link_fields' => _flag_link_type_fields(),
    '#after_build' => array('flag_expand_link_option'),
    '#default_value' => variable_get('flag_lists_link_type', NULL),
  );
  $form['rebuild'] = array(
    '#type' => 'fieldset',
    '#title' => t('Global rebuild'),
    '#description' => t('Changes to these settings normally apply to only newly
      created flags. However, you can globally apply changes here. First adjust
      the settings above and save them. Then click the link below. This will
      change all existing flag lists. It cannot be undone.'),
    '#tree' => FALSE,
  );
  $form['rebuild']['global_rebuild'] = array(
    '#value' => l('Rebuild all flag lists.', 'admin/build/flags/lists/rebuild'),
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
function flag_lists_delete_confirm(&$form_state, $fid) {
  $flag = flag_lists_get_flag($fid);
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
    $flag->delete();
    $flag->disable();
    _flag_lists_clear_cache();
  }
  drupal_set_message(t('@title has been deleted.', array('@title' => $flag->get_title())));
}