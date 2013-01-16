<?php
interface Drush_SearchReplaceDb {
  public function getTables();
  public function getColumns($tableName);
  public function getSearchIterator($tableName, $columnName, $search);
}

interface Drush_SearchReplaceDb_Iterator {
  public function next();
  public function updateCurrentRow($new);
  public function updateCurrentRowWithTextReplace($replace);
}
