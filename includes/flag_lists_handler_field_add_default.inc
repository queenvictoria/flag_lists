<?php
/**
 * @file
 * Contains the ... field handler.
 */

/**
 * Field handler to provide
 */
class flag_lists_handler_field_add_default extends views_handler_field {
  /**
   * Render whatever the data is as a link to the list.
   *
   * Data should be made XSS safe prior to calling this function.
   */


  /**
   * Overriden to add the field for the entity id.
   */
  function query() {
    $this->table_alias = $base_table = $this->view->base_table;
    $this->base_field = $this->view->base_field;

    if (!empty($this->relationship)) {
      foreach ($this->view->relationship as $relationship) {
        if ($relationship->alias == $this->relationship) {
          $base_table = $relationship->definition['base'];
          $this->table_alias = $relationship->alias;

          $table_data = views_fetch_data($base_table);
          $this->base_field = empty($relationship->definition['base field']) ? $table_data['table']['base']['field'] : $relationship->definition['base field'];
        }
      }
    }

    // Add the field if the query back-end implements an add_field() method,
    // just like the default back-end.
    if (method_exists($this->query, 'add_field')) {
      $this->field_alias = $this->query->add_field($this->table_alias, $this->base_field, '');
    }

    $this->add_additional_fields();
  }

  function render($values) {
    $entity_id = $values->entity;
    return flag_lists_default_link($entity_id, FALSE);
  }
}
