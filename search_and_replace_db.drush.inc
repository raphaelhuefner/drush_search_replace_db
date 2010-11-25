<?php
// $Id:$

/**
* @File Search and Replace DB Drush Plugin
*/

//require_once (dirname(__FILE__) . '/drush_search_and_replace_db.class.inc');

/**
 * Implementation of hook_drush_command().
 */
function drush_search_and_replace_db_drush_command() {
  $items['search-and-replace-db'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_DATABASE,
    'description' => dt('Searches and replaces text in the whole DB.'),
    'callback' => 'drush_search_and_replace_db_execute',
//    'arguments' => array(
//      'searchstring' => dt('Required. At least one instance required. String to be searched.'),
//      'replacestring' => dt('Required. At least one instance required. String acting as a replacement.'),      
//    ),    
/*
    'options' => array(
      '--search=searchstring' => 'String to be searched.',
      '--replace=replacestring' => 'String acting as a replacement.',
    ),
*/

  );

  return $items;
}

/**
 * Implementation of hook_drush_help().
 */
function drush_search_and_replace_db_drush_help($section) {
  switch ($section) {
    case 'drush:search-and-replace-db':
      return dt("drush search-and-replace-db --search searchstring --replace replacestring [--search searchstring --replace replacestring [--search searchstring --replace replacestring [...]]]");
  }
}

/**
 * Drush callback; make based on the makefile.
 */
function drush_search_and_replace_db_execute() {
  $args = func_get_args();
  var_dump($args);

//  var_dump(drush_get_option('search'));
//  var_dump(drush_get_option('replace'));
  
  $data['hiho'] ='there';
  drush_print('Ready.');
  drush_log(dt('Search and replace information "%data" processed.', array('%data' => $data['hiho'])), 'ok');
  
//  exit;
//  drush_set_error('FEATURES_CLONE_INVALID_FEATURENAME_SRC', dt('Invalid source feature name. No module found with such name.' . $please_refer));
  
//  $cloner = new features_cloner();
/*  
  $ret = $cloner->execute(func_get_args());
  
  if ($ret === FALSE) {
    print("Aborting.\n");
    return;
  }  
*/
}



function drush_search_and_replace_db_update_db(array $search_and_replace_patterns=array()) {
  global $db_prefix;

  if (empty($search_and_replace_patterns)) {
    $search_and_replace_patterns = array(
      'sites/all/modules/cantrust' => 'profiles/gcsc/modules/contrib',
      'sites/all/modules/contrib' => 'profiles/gcsc/modules/contrib',
      'sites/all/modules/custom' => 'profiles/gcsc/modules/custom',
      'sites/all/modules/developer' => 'profiles/gcsc/modules/development',
      'sites/all/modules/features' => 'profiles/gcsc/modules/features',
      'sites/all/themes/contrib' => 'profiles/gcsc/themes',
      'sites/all/themes/custom' => 'profiles/gcsc/themes',
      'sites/all/libraries' => 'profiles/gcsc/libraries',
    );
  }

  $update_sql = '';
  if (! is_string($db_prefix)) {
    // @todo take care of $db_prefix
    die('Cannot handle complex $db_prefix setting.');
  }
  $tables_result = db_query("SHOW TABLES LIKE '%s%%'", $db_prefix);
  while ($table_name = db_result($tables_result)) {
//    var_dump('TABLE ' . $table_name);
    $columns_result = db_query("SHOW COLUMNS FROM %s", $table_name);
    while ($column = db_fetch_object($columns_result)) {
      $column_name = $column->Field;
//      var_dump('  COLUMN ' . $table_name . '.' . $column_name);
      foreach ($search_and_replace_patterns as $search => $replace) {
        $column_values_result = db_result(db_query("SELECT %s AS column_value FROM %s WHERE %s LIKE '%%%s%%';", $column_name, $table_name, $column_name, $search));
        while ($column_value = db_result($column_values_result)) {
          $unserialized_column_value = unserialize($column_value);
          if ((FALSE !== $unserialized_column_value) || (serialize(FALSE) == $column_value)){
            $value_as_php_code = var_export($unserialized_column_value, TRUE);
            $changed_value_as_php_code = str_replace($search, $replace, $value_as_php_code);
            $changed_value = eval($changed_value_as_php_code);
            $changed_column_value = serialize($changed_value);
            db_query("UPDATE %s SET %s = '%s' WHERE %s = '%s';", $table_name, $column_name, $changed_column_value, $column_name, $column_value);
          }
          else {
            db_query("UPDATE %s SET %s = REPLACE(%s, '%s', '%s') WHERE %s = '%s';", $table_name, $column_name, $column_name, $search, $replace, $column_name, $column_value);
          }
        }
      }
    }
  }
  db_query("UPDATE variable SET value = 's:4:\"gcsc\";' WHERE value = 's:7:\"default\";' AND name IN ('install_profile', 'backup_migrate_profile_id');\n");
}


