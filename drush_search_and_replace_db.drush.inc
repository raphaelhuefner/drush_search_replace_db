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

