<?php

class DbConnection {
  private static $connection;

  public static function getConnection() {
    if (!self::$connection) {
            $servername = "domain";
            $dbname = "dbname";
            $username = "USER";
            $password = "PASS";

      $hostname = "p:" . $servername; // Prepend "p:" for persistent connection

      self::$connection = new mysqli($hostname, $username, $password, $dbname);

      if (self::$connection->connect_error) {
        die("Connection failed: " . self::$connection->connect_error);
      }
    }

    return self::$connection;
  }
}

// Function to display PHP version

$sensorDataArray = [];

$query = "SELECT * FROM sensorreading ORDER BY timestamp DESC LIMIT 5";

// Get persistent connection using DbConnection class
$conn = DbConnection::getConnection();

// Execute the query repeatedly within a loop (assuming live.html fetches every second)
  $result = $conn->query($query);

  if ($result->num_rows > 0) {
    $sensorDataArray = []; // Reset array for new data
    while ($row = $result->fetch_assoc()) {
      $sensorData = array(
        'spo2' => $row['spo2'],
        'heartRate' => $row['heartRate'],
        'timestamp' => $row['timestamp']
      );
      $sensorDataArray[] = $sensorData;
    }
    echo json_encode($sensorDataArray);
  } else {
    echo json_encode(array('error' => 'No data available'));
  }

  // Optional: Implement a sleep timer to control query frequency (adjust in seconds)
  sleep(1);

  // Free the result from memory
  $result->free_result();


// No need to close the connection explicitly (handled by persistent connection)

// Display PHP version for debugging purposes (remove for production)

?>