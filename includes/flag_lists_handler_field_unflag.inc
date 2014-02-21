<?php
/**
 * @file
 * Contains the ... field handler.
 */

/**
 * Field handler to provide
 */
class flag_lists_handler_field_unflag extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['uid'] = 'uid';
    $this->additional_fields['fid'] = 'fid';
    $this->additional_fields['entity_id'] = 'entity_id';
   }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['text'] = array('default' => '', 'translatable' => TRUE);
    return $options;
  }


  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['text'] = array(
      '#type' => 'textfield',
      '#title' => t('Text for link'),
      '#default_value' => $this->options['text'],
    );
  }


  function render($values) {
    $uid = $values->{$this->aliases['uid']};
    $fid = $values->{$this->aliases['fid']};
    $entity_id = $values->{$this->aliases['entity_id']};
    $text = !empty($this->options['text']) ? $this->options['text'] : NULL;

    $link = flag_lists_get_link($uid, $fid, $entity_id, $text, 'unflag');
    return $link;
  }

}
