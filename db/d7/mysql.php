<?php
class Drush_SearchReplaceDb_D7_Mysql implements Drush_SearchReplaceDb {

  /**
   * List all Drupal tables in database
   *
   * @throws Exception
   */
  public function getTables() {
    return db_query("SHOW TABLES LIKE '{%}'")->fetchCol();
  }

  public function getColumns($tableName) {
    return db_query("SHOW COLUMNS FROM `$tableName`")->fetchCol(0); // Column 0 should be the `Field` column.
  }

  public function getSearchIterator($tableName, $columnName, $search) {
    $query = db_query("SELECT `$columnName` AS column_value FROM `$tableName` WHERE `$columnName` LIKE :search;", array(':search' => '%' . $search . '%'));
    return new Drush_SearchReplaceDb_D7_Mysql_Iterator($query);
  }

  public function update($tableName, $columnName, $original, $new) {
    db_query("UPDATE `$tableName` SET `$columnName` = :new WHERE `$columnName` = :original;", array(':original' => $original, ':new' => $new));
  }

  public function updateWithTextReplace($tableName, $columnName, $original, $search, $replace) {
    db_query("UPDATE `$tableName` SET `$columnName` = REPLACE(`$columnName`, :search, :replace) WHERE `$columnName` = :original;", array(':original' => $original, ':search' => $search, ':replace' => $replace));
  }
}

class Drush_SearchReplaceDb_D7_Mysql_Iterator implements Drush_SearchReplaceDb_Iterator {

  protected $query = null;

  public function __construct(DatabaseStatementInterface $query) {
    $this->query = $query;
  }

  public function next() {
    return $this->query->fetchField(0);
  }
}
