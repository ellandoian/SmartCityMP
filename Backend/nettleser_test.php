<?php
//KILDE: Namoshek. (24/4/2024) php-mqtt/client (Versjon 2.1.0) [Programvare] https://github.com/php-mqtt/client Hentet (7/6/2024)
//KILDE: Guowei, Li. (5.8.2021) How to use MQTT in PHP https://www.emqx.com/en/blog/how-to-use-mqtt-in-php Hentet (20/5/2024)
// Må inkludere vendor ettersom det er der php-bibliotekene lagres
require 'vendor/autoload.php';

// Importerer alle bibliotek
use PhpMqtt\Client\MqttClient;
use PhpMqtt\Client\ConnectionSettings;

// Oppretter en rekke variabler for å kunne koble seg på MQTT
$server   = '10.25.18.138';
$port     = 1883;
$clientId = 'PHP_Client';
$username = 'njaal';
$password = '3Inshallah4';
$topic    = 'pytophp';

//$jsonFile = 'data.json';
//if (file_exists($jsonFile)) {
//    $data = json_decode(file_get_contents($jsonFile), true);
//} else {
//    $data = [];
//}

//$dijkstra = file_get_contents('data.txt');
//echo $dijkstra;

try {
    // Lager en ny instans av MQTT client
    $mqtt = new MqttClient($server, $port, $clientId);
    $connectionSettings = (new ConnectionSettings)
        ->setUsername($username)
        ->setPassword($password);
    // Kobler seg til MQTT brokeren
    $mqtt->connect($connectionSettings, true);
    echo "Oprettet tilkobling til Broker .\n";

    // Abonnerer til topic 'pytophp'
    $mqtt->subscribe($topic, function ($topic, $message) {
        printf("Melding er blitt motatt på topic [%s]: %s\n", $topic, $message);

        // Programmet skjekker at data.json eksisterer
        $jsonFile = 'data.json';
        if (file_exists($jsonFile)) {
            // Dersom filen eksisterer er $data variabelen lik inneholdet i filen:
            $data = json_decode(file_get_contents($jsonFile), true);
        } else {
            // Hvis ikke er det en tom liste
            $data = [];
        }
        // Her filtreres ut uønskede meldinger,
        // grunnen til at meldinger som starter med '{'
        // filtreres ut er at det konstant ble publisert en
        // melding som startet med dette tegnet, som vi ikke
        // ønsket å legge til i tabellen til brukeren på nettsiden
        if(str_starts_with($message, "{")) {
            echo "   Filtrert ut";
        } else {
            // Hvis ikke meldingen starter med '{', legges meldingen og et tidsstempel til i .json filen
            // Her er ikke biltypen avhengig av noe annet, den ble for enkelhetsskyld bestemt til å være
            // en diesel bil
            $nydata = ['tidspunkt' => date("h:i:sa"), 'belop' => $message, 'Biltype' => 'Diesel'];
            $data[] = $nydata;
            file_put_contents($jsonFile, json_encode($data, JSON_PRETTY_PRINT));
        }}, 0);
        //$payload = $dijkstra;
        //$cleanedPayload = trim($payload);
        //$mqtt->publish('phptozumo', json_encode($cleanedPayload), 0, true);

    // Vi ønsker at programmet skal konstant være i MQTT-loopen
    // slik at det alltid kan motta meldinger mens det kjører
    $mqtt->loop(true);
    //$mqtt->disconnect();
// til slutt en catch i tilfelle en feil skal oppstå
} catch (\Exception $e) {
    echo sprintf("An error occurred: %s\n", $e->getMessage());
}
?>
