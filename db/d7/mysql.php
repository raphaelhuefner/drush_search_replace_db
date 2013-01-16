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
    $query = db_query("SELECT * FROM `$tableName` WHERE `$columnName` LIKE :search;", array(':search' => '%' . $search . '%'));
    return new Drush_SearchReplaceDb_D7_Mysql_Iterator($tableName, $columnName, $search);
  }
}

class Drush_SearchReplaceDb_D7_Mysql_Iterator implements Drush_SearchReplaceDb_Iterator {
  protected $tableName = '';
  protected $columnName = '';
  protected $search = '';
  protected $currentRow = array();
  protected $query = null;

  public function __construct($tableName, $columnName, $search) {
    $this->tableName = $tableName;
    $this->columnName = $columnName;
    $this->search = $search;
    $this->query = db_query("SELECT * FROM `$tableName` WHERE `$columnName` LIKE :search;", array(':search' => '%' . $search . '%'));
  }

  public function next() {
    $this->currentRow = $this->query->fetchAssoc();
    return isset($this->currentRow[$this->columnName]) ? $this->currentRow[$this->columnName] : FALSE;
  }

  public function updateCurrentRow($new) {
    $update = db_update($this->tableName);
    $update->fields(array($this->columnName => $new));
    foreach ($this->currentRow as $fieldName => $fieldValue) {
      $update->condition($fieldName, $fieldValue);
    }
    $update->execute();
  }

  public function updateCurrentRowWithTextReplace($replace) {
    $update = db_update($this->tableName);
    $update->expression(
      $this->columnName,
      'REPLACE(`' . $this->columnName . '`, :search, :replace)',
      array(
        ':search' => $this->search,
        ':replace' => $replace
      )
    );
    foreach ($this->currentRow as $fieldName => $fieldValue) {
      $update->condition($fieldName, $fieldValue);
    }
    $update->execute();
  }
}
