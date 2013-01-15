<?php
// $Id:$

/**
 * @file Search and replace DB Drush Plugin
 * @license http://opensource.org/licenses/gpl-3.0.html GNU General Public License, version 3 (GPLv3)
 * @author Raphael Huefner http://www.raphaelhuefner.com
 * Sponsored by Affinity Bridge http://www.affinitybridge.com
 */

require_once(dirname(__FILE__) . '/SearchReplaceSerialized.class.php');
require_once(dirname(__FILE__) . '/db/interfaces.php');

/**
 * Implements hook_drush_command().
 */
function search_replace_db_drush_command() {
  $items = array();
  $items['search-replace-db'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_DATABASE,
    'description' => dt('Searches and replaces text in the whole DB.'),
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
 * Drush command validation.
 */
function drush_search_replace_db_get_db_handler() {
  static $db_handler = NULL;
  if (! $db_handler) {
    $drupal_major_version = drush_drupal_major_version();
    switch ($drupal_major_version) {
      case '6':
        global $db_type;
        $db_driver = $db_type;
        break;
      case '7':
        // fall-through intended.
      default:
        $db_driver = db_driver();
        break;
    }
    $potential_db_handler_filename = dirname(__FILE__) . '/db/d' . $drupal_major_version . '/' . $db_driver . '.php';
    if (file_exists($potential_db_handler_filename)) {
      require_once($potential_db_handler_filename);
      $classname = 'Drush_SearchReplaceDb_D' . $drupal_major_version . '_' . ucfirst($db_driver);
      if (! class_exists($classname)) {
        throw new Exception(dt('Expected class "!classname" to be defined in file "!filename"', array('!classname' => $classname, '!filename' => $potential_db_handler_filename)));
      }
      $db_handler = new $classname();
    }
    else {
      throw new Exception(dt('Can\'t find a DB Handler for this combination of Drupal version and DB type'));
    }
  }
  return $db_handler;
}

/**
 * Drush command validation.
 */
function drush_search_replace_db_validate() {
  try {
    $db = drush_search_replace_db_get_db_handler();
  }
  catch (Exception $e) {
    return drush_set_error('SEARCH_REPLACE_DB_NO_DB_HANDLER', $e->getMessage());
  }
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
if (! defined('ESC_RESET')) define('ESC_RESET', chr(0x1B) . '[0m');
if (! defined('ESC_BRIGHT_RED')) define('ESC_BRIGHT_RED', chr(0x1B) . '[1;31m');
if (! defined('ESC_BRIGHT_GREEN')) define('ESC_BRIGHT_GREEN', chr(0x1B) . '[1;32m');

/**
 * Drush command callback. This is where the action takes place.
 */
function drush_search_replace_db() {
  try {
    $args = func_get_args();
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
  catch (Exception $e) {
    return drush_set_error('SEARCH_REPLACE_ERROR', $e->getMessage());
  }
}

function search_replace_db_is_serialized($string) {
  $parser = new SearchReplaceSerialized($string, 'x', 'x');
  try {
    $dummy_output = $parser->run();
  }
  catch (Exception $e) {
    return FALSE;
  }
  return TRUE;
}

/**
 * Actual business logic.
 */
function search_replace_db_update_db(array $search_replace_patterns=array()) {
  $db = drush_search_replace_db_get_db_handler();
  foreach ($db->getTables() as $table_name) {
    drush_log(dt('TABLE @table_name', array('@table_name' => $table_name)), 'debug');
    foreach ($db->getColumns($table_name) as $column_name) {
      drush_log(dt('  COLUMN @table_name.@column_name', array('@table_name' => $table_name, '@column_name' => $column_name)), 'debug');
      foreach ($search_replace_patterns as $search => $replace) {
        drush_log(dt('    SEARCHING FOR "@search"', array('@search' => $search)), 'debug');
        $iterator = $db->getSearchIterator($table_name, $column_name, $search);
        while ($column_value = $iterator->next()) {
          if (search_replace_db_is_serialized($column_value)) {
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
            $db->update($table_name, $column_name, $column_value, $changed_column_value);
          }
          else {
            drush_log(dt('Found "@search" and replace with "@replace" in table "@table" in column "@column".', array('@search' => $search, '@replace' => $replace, '@table' => $table_name, '@column' => $column_name)), 'info');
            $db->updateWithTextReplace($table_name, $column_name, $column_value, $search, $replace);
          }
        }
      }
    }
  }
}
