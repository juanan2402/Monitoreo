const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');

const http = require('http');
const express = require('express');
const socketIO = require('socket.io');
const app = express();
const server = http.createServer(app);
const io = socketIO(server);

server.listen(3000, function () {
    console.log('Server listening on port', 3000);
});

app.use(express.static(__dirname + '/public'));

const port = new SerialPort({
    path: 'COM5', 
    baudRate: 115200,
});

const parser = port.pipe(new ReadlineParser({ delimiter: '\r\n' }));

port.on('open', () => {
    console.log('Serial port opened');
});

let lecturaActiva = false;
let temperatura1 = null;
let humedad1 = null;
let temperatura2 = null;
let humedad2 = null;
let temperatura3 = null;
let humedad3 = null;
let temperatura4 = null;
let humedad4 = null;
let temperatura5 = null;
let humedad5 = null;
let temperatura6 = null;
let humedad6 = null;
let activo = 0;
let contadorPulsadores = [0, 0, 0, 0];

// Event for when the serial port receives data
parser.on('data', (data) => {
    console.log(`Message from ESP32: ${data}`);
    try {
        if (data === 'block') {
            lecturaActiva = !lecturaActiva;

            if (lecturaActiva) {
                console.log("Lectura activada.");
                activo = 1;
            } else {
                console.log("Lectura desactivada.");
                activo = 0;
                io.emit('sensorData', { activo });
            }
        }
        if (lecturaActiva) {
            const valores = data.split(',');

            if (valores.some((valor) => valor.includes('.'))) {
                // Temperature and humidity data
                temperatura1 = parseFloat(valores[0]);
                humedad1 = parseFloat(valores[1]);
                temperatura2 = parseFloat(valores[2]);
                humedad2 = parseFloat(valores[3]);
                temperatura3 = parseFloat(valores[4]);
                humedad3 = parseFloat(valores[5]);
                temperatura4 = parseFloat(valores[6]);
                humedad4 = parseFloat(valores[7]);
                temperatura5 = parseFloat(valores[8]);
                humedad5 = parseFloat(valores[9]);
                temperatura6 = parseFloat(valores[10]);
                humedad6 = parseFloat(valores[11]);

                console.log(`Temperature: ${temperatura1}°C, Humidity: ${humedad1}%`);

                // Emit temperature and humidity data
                io.emit('sensorData', {
                    temperatura1,
                    humedad1,
                    temperatura2,
                    humedad2,
                    temperatura3,
                    humedad3,
                    temperatura4,
                    humedad4,
                    temperatura5,
                    humedad5,
                    temperatura6,
                    humedad6,
                    contadorPulsadores,
                    activo
                });
            } else {
                contadorPulsadores = valores.map((valor) => parseInt(valor, 10));
                console.log(`Button counters: ${contadorPulsadores.join(', ')}`);
                // No emitimos datos aquí porque son de contadores.
            }
        }
    } catch (error) {
        console.error('Error processing data:', error);
    }
});
