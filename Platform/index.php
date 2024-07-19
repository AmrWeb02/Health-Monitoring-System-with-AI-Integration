<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Document</title>
</head>
<body>
    <h1>test</h1>
</body>
</html>

<?php
$servername = "domain";
$dbname = "dbname";
$username = "username";
$password = "pass";
$api_key_value = "a";
$api_key;
$spo2;
$heartRate;
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $api_key = $_POST["api_key"];
    if ($api_key == $api_key_value) {
        $spo2 = $_POST["spo2"];
        $heartRate = $_POST["heartRate"];
        $hostname = "p:" . $servername;
        $conn = new mysqli($hostname, $username, $password, $dbname);
        // Check connection
        if ($conn->connect_error) {
            die("Connection failed: " . $conn->connect_error);
        }

        $sql = "INSERT INTO sensorreading (spo2, heartRate) 
                VALUES ('" . $spo2 . "', '" . $heartRate . "')";

        if ($conn->query($sql) === TRUE) {
            echo "New record created successfully";
        } else {
            echo "Error: " . $sql . "<br>" . $conn->error;
        }
    } else {
        echo "Wrong API Key provided.";
    }
} else {
    echo "No data posted with HTTP POST.";
}
?>