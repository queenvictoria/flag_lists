<?php
// $I$
/**
 * @file
 * Contains the basic 'list count' field handler.
 */

/**
 * Field handler to provide simple renderer that counts the number of entities in a flag list.
 */
class flag_lists_handler_field_list_count extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['fid'] = 'fid';
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['text'] = array('default' => '', 'translatable' => TRUE);

    return $options;
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function render($values) {
    $fid = $values->{$this->aliases['fid']};
    $count = _flag_lists_get_list_count($fid);
    return $count;
  }
}
