<?php
interface Drush_SearchReplaceDb {
  public function getTables();
  public function getColumns($tableName);
  public function getSearchIterator($tableName, $columnName, $search);
  public function update($tableName, $columnName, $original, $new);
  public function updateWithTextReplace($tableName, $columnName, $original, $search, $replace);
}

interface Drush_SearchReplaceDb_Iterator {
  public function next();
}
