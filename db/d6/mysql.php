<?php
class Drush_SearchReplaceDb_D6_Mysql implements Drush_SearchReplaceDb {

  /**
   * List all Drupal tables in database
   *
   * @throws Exception
   */
  public function getTables() {
    $table_names = array();
    $tables_result = db_query("SHOW TABLES LIKE '%s%%'", $db_prefix);
    while ($table_name = db_result($tables_result)) {
      $table_names[] = $table_name;
    }
    return $table_names;
  }

  public function getColumns($tableName) {
    $column_names = array();
    $columns_result = db_query("SHOW COLUMNS FROM `%s`", $tableName);
    while ($column = db_fetch_object($columns_result)) {
      $column_names[] = $column->Field;
    }
    return $column_names;
  }

  public function getSearchIterator($tableName, $columnName, $search) {
    return new Drush_SearchReplaceDb_D6_Mysql_Iterator($tableName, $columnName, $search);
  }

  public function update($tableName, $columnName, $original, $new) {
    db_query("UPDATE `%s` SET `%s` = '%s' WHERE `%s` = '%s';", $tableName, $columnName, $new, $columnName, $original);
  }

  public function updateWithTextReplace($tableName, $columnName, $original, $search, $replace) {
    db_query("UPDATE `%s` SET `%s` = REPLACE(`%s`, '%s', '%s') WHERE `%s` = '%s';", $tableName, $columnName, $columnName, $search, $replace, $columnName, $original);
  }
}

class Drush_SearchReplaceDb_D6_Mysql_Iterator implements Drush_SearchReplaceDb_Iterator {
  protected $tableName = '';
  protected $columnName = '';
  protected $search = '';
  protected $currentRow = array();
  protected $query = null;

  public function __construct($tableName, $columnName, $search) {
    $this->tableName = $tableName;
    $this->columnName = $columnName;
    $this->search = $search;
    $this->query = db_query("SELECT * FROM `%s` WHERE `%s` LIKE '%%%s%%';", $tableName, $columnName, $search);
  }

  public function next() {
    $this->currentRow = db_fetch_array($this->query);
    return isset($this->currentRow[$this->columnName]) ? $this->currentRow[$this->columnName] : FALSE;
  }

  public function updateCurrentRow($new) {
    $sql = "UPDATE `%s` SET `%s` = '%s'";
    $query_args = array(
      $this->tableName,
      $this->columnName,
      $new
    );
    $this->executeUpdate($sql, $query_args);
  }

  public function updateCurrentRowWithTextReplace($replace) {
    $sql = "UPDATE `%s` SET `%s` = REPLACE(`%s`, '%s', '%s')";
    $query_args = array(
      $this->tableName,
      $this->columnName,
      $this->columnName,
      $this->search,
      $replace
    );
    $this->executeUpdate($sql, $query_args);
  }

  protected function executeUpdate($sql, array $query_args) {
    $sql_delim = " WHERE ";
    foreach ($this->currentRow as $fieldName => $fieldValue) {
      $query_args[] = $fieldName;
      $query_args[] = $fieldValue;
      $sql .= $sql_delim . " (`%s` = '%s') ";
      $sql_delim = " AND ";
    }
    db_query($sql, $query_args);
  }
}
