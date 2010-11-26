<?php
// $Id:$

/**
* @File Search and Replace DB Drush Plugin
*/

require_once (dirname(__FILE__) . '/SearchReplaceSerialized.class.php');

/**
 * Implements hook_drush_command().
 */
function search_replace_db_drush_command() {
  $items['search-replace-db'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_DATABASE,
    'description' => dt('Searches and replaces text in the whole DB.'),
    'callback' => 'search_replace_db_execute',
  );
  return $items;
}

/**
 * Implements hook_drush_help().
 */
function search_replace_db_drush_help($section) {
  switch ($section) {
    case 'drush:search-replace-db':
      return dt("drush search-replace-db searchstring replacestring [searchstring replacestring [searchstring replacestring [...]]]");
  }
}

/**
 * X.
 */
function search_replace_db_validate() {
  $args = func_get_args();
  if (0 == count($args)) {
    return drush_set_error('SEARCH_REPLACE_DB_NO_DATA', dt('At least one search/replace pair must be given to make sense of this command at all.'));
  }
  if (0 != (count($args) % 2)) {
    return drush_set_error('SEARCH_REPLACE_DB_ODD_DATA', dt('Odd number of arguments. Maybe missing replace text for last search pattern?'));
  }
  return TRUE;
}

// <ESC>[{attr};{fg};{bg}m 
// @see http://linuxgazette.net/issue65/padala.html
define('ESC_RESET', chr(0x1B) . '[0m');
define('ESC_BRIGHT_RED', chr(0x1B) . '[1;31m');
define('ESC_BRIGHT_GREEN', chr(0x1B) . '[1;32m');

/**
 * Drush command callback. This is where the action takes place.
 */
function search_replace_db_execute() {
  $args = func_get_args();
  if (! call_user_func_array('search_replace_db_validate', $args)) {
    return drush_set_error('SEARCH_REPLACE_GENERAL_ERROR', dt('Search/replace parameter validation error.'));
  }
  $search_replace_patterns = array();
  for ($i = 0; $i < count($args); $i+=2) {
    $search_replace_patterns[$args[$i]] = $args[$i+1];
  }
  foreach ($search_replace_patterns as $search => $replace) {
    drush_print(ESC_RESET . ESC_BRIGHT_RED . $search . ESC_RESET . ' => ' . ESC_BRIGHT_GREEN . $replace . ESC_RESET);
  }
  if (drush_confirm('Do you really want to do that?')) {
    drush_log(dt('Ok, doing it.'), 'info');
    return search_replace_db_update_db($search_replace_patterns);
  }
}

/**
 * Actual business logic.
 */
function search_replace_db_update_db(array $search_replace_patterns=array()) {
  global $db_prefix;

  if (! is_string($db_prefix)) {
    // @todo take care of complex/array $db_prefix
    return drush_set_error('SEARCH_REPLACE_UNIMPLEMENTED_ERROR', dt('Cannot handle complex/array $db_prefix setting.'));
  }
  $tables_result = db_query("SHOW TABLES LIKE '%s%%'", $db_prefix);
  while ($table_name = db_result($tables_result)) {
    drush_log(dt('TABLE %table_name', array('%table_name' => $table_name)), 'debug');
    $columns_result = db_query("SHOW COLUMNS FROM %s", $table_name);
    while ($column = db_fetch_object($columns_result)) {
      $column_name = $column->Field;
      drush_log(dt('  COLUMN %table_name.%column_name', array('%table_name' => $table_name, '%column_name' => $column_name)), 'debug');
      foreach ($search_replace_patterns as $search => $replace) {
        drush_log(dt('    SEARCHING FOR "%search"', array('%search' => $search)), 'debug');
        $column_values_result = db_query("SELECT %s AS column_value FROM %s WHERE %s LIKE '%%%s%%';", $column_name, $table_name, $column_name, $search);
        while ($column_value = db_result($column_values_result)) {
          $unserialized_column_value = unserialize($column_value);
          if ((FALSE !== $unserialized_column_value) || (serialize(FALSE) == $column_value)){
            drush_log(dt('Found "@search", start replacing with "@replace" in PHP serialized data in table "@table" in column "@column".', array('@search' => $search, '@replace' => $replace, '@table' => $table_name, '@column' => $column_name)), 'info');
            $start_time = microtime(TRUE);
            $parser = new SearchReplaceSerialized($column_value, $search, $replace);
            try {
              $changed_column_value = $parser->run();
            }
            catch (Exception $e) {
              return drush_set_error('SEARCH_REPLACE_PARSER_ERROR', dt('SearchAndReplaceSerialized Parser exception: @exception_message', array('@exception_message' => $e->getMessage())));
            }
            drush_log(dt('End of search and replace in PHP serialized data. Runtime was @secs seconds.', array('@secs' => microtime(TRUE) - $start_time)), 'info');
            db_query("UPDATE %s SET %s = '%s' WHERE %s = '%s';", $table_name, $column_name, $changed_column_value, $column_name, $column_value);
          }
          else {
            drush_log(dt('Found "@search" and replace with "@replace" in table "@table" in column "@column".', array('@search' => $search, '@replace' => $replace, '@table' => $table_name, '@column' => $column_name)), 'info');
            db_query("UPDATE %s SET %s = REPLACE(%s, '%s', '%s') WHERE %s = '%s';", $table_name, $column_name, $column_name, $search, $replace, $column_name, $column_value);
          }
        }
      }
    }
  }
}
