<!DOCTYPE html>
// KILDE: Alaeddin, Amama. (27/2/2017) $_POST vs. $_SERVER[‘REQUEST_METHOD’] == ‘POST’ 
// https://stackoverflow.com/questions/409351/post-vs-serverrequest-method-post Hentet (31/5/2024)
// KILDE: W3schools. (2024) PHP Superglobal -$GET https://www.w3schools.com/php/php_superglobals_get.asp Hentet (4/6/2024)
<html>
<head>
    // Overskrift
    <title>Planlegg din rute her.</title>
</head>
<head>
    <h1> Planlegg din rute her </h1>
        // Legger inn bilde av kartet
        <img src="/kartet.png" alt="Image" width="600">
</head>
<body>

<?php
// Bruker her en superglobal variabel $_SERVER, 'Serverwide' global variabel
// Venter til brukerens input har blitt "submitted"
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    // henter brukerens input og lagrer det i to variabler
    $startposisjon = $_POST['startposisjon'];
    $sluttposisjon = $_POST['sluttposisjon'];

    // Tømmer data.txt
    $file = './data.txt';
    // Legger til de nye inputtene fra brukeren
    file_put_contents($file, "$startposisjon,$sluttposisjon\n");
    // echo for å gi beskjed til brukeren av nettsiden at informasjonen har blitt sendt til bilen
    echo "<p>Din rute har blitt sendt til bilen.</p>";
}
?>
// Form genererer felt som brukeren kan fylle inn
// Inputtene til brukeren blir ikke sendt før submit knappen trykkes
<form method="post" action="">
  <label for="startposisjon">Din startposisjon:</label>
  // Gir inputtene ID og navn for å kunne lagre inputtene som variabler i PHP
  <input type="text" id="startposisjon" name="startposisjon"><br><br>
  <label for="sluttposisjon">Din sluttposisjon:</label>
  <input type="text" id="sluttposisjon" name="sluttposisjon"><br><br>
  <input type="submit" value="Submit">
</form>
</body>
</html>