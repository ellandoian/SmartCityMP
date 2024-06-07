// KILDE: Szot, Kamil. (16/4/2013). Creating a table with PHP foreach function 
// https://medium.com/personal-work-revision/how-to-parse-json-into-html-table-with-php-for-dummies-1da2b4b7db55 Hentet (4/6/2024)

<?php
// henter data fra data.json
$jsonFile = 'data.json';
$data = [];
if (file_exists($jsonFile)) {
    // Lagrer innholdet i data.json i variabelen $data
    $data = json_decode(file_get_contents($jsonFile), true);
}
?>

<!DOCTYPE html>
<html>
<head>
    <title>Mitt dashbord:</title>
</head>
<body>
    // Bilde legges inn
    <h3>Min side:</h3>
    <img src="/BIELSYS.png" alt="Image" width="300">
</body>
<br>
</body>
</html>

<head>
    // Charset bestemmer hvordan karakterer skal kodes i HTML-dokumentet
    <meta charset="UTF-8">
    <title>Mitt Dashboard</title>
    <style>
        // Konfigurerer hvordan tabellen skal se ut
        table {
            width: 100%;
            border-collapse: collapse;
        }
        table, th, td {
            border: 1px solid black;
        }
        th, td {
            padding: 10px;
            text-align: left;
        }
    </style>
</head>
<body>
    // Overskrift
    <h1>Mitt dashbord</h1>
    <table>
        <thead>
            // Genererer den øverste radene i tabellen
            <tr>
                <th>Tidspunkt</th>
                <th>Beløp</th>
                <th>Biltype</th>
            </tr>
        </thead>
        <tbody>
        <script>
                // Denne funksjonen gjør at siden oppdateres hvert 50. sekund
                function autoRefresh() {
                    window.location = window.location.href;
                }
                setInterval('autoRefresh()', 50000);
            </script>
            // Denne koden bruker $data-variabelen til å legge til rader i tabellen
            <?php if (!empty($data)): ?>
                // For each funksjonen itererer gjennom verdiene i $data og legger de til
                <?php foreach ($data as $row): ?>
                    <tr> // Bruker 'tidspunkt', 'belop' og 'Biltype' for å plassere verdiene i riktig kolonne
                        <td><?php echo htmlspecialchars($row['tidspunkt']); ?></td>
                        <td><?php echo htmlspecialchars($row['belop']); ?> kr</td>
                        <td><?php echo htmlspecialchars($row['Biltype']); ?></td>
                    </tr>
                <?php endforeach; ?>
            <?php else: ?>
                <tr>
                    <td colspan="3">No data available.</td>
                </tr>
            <?php endif; ?>
        </tbody>
    </table>
</body>
</html>