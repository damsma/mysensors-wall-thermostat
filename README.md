# mysensors-wall-thermostat
more complex description coming soon<br><br>


<h2>Screenshots</h2>
<h3>Home Assistant</h3>

![grafik](https://user-images.githubusercontent.com/45398732/204623008-7a077056-5cc6-44b4-a956-9ec1a31a5fc1.png)

<h3>Domoticz</h3>

![grafik](https://user-images.githubusercontent.com/45398732/204621900-5fa87f26-3f23-46d6-b87d-4fd7a6a75c5f.png)
![grafik](https://user-images.githubusercontent.com/45398732/204622015-3d09dd9c-9d6c-4e16-b8c0-beaf6bfd64e9.png)



<h2>Wiring diagram</h2>

OLED 1.5", SPI/I2C interfaces, 16-bit grey level:<br>
VCC		->	5V<br>
GND		->  GND<br>
DIN		->	D3<br>
CLK     ->	D2<br>
CS      ->	D4<br>
DC      ->	D5<br>
RST     ->	D6<br><br>

SHT-30:<br>
VIN     ->	3.3V<br>
GND     ->	GND<br>
SCL     ->	D21<br>
SAA     ->	D20<br><br>

W5100:<br>
V+5     ->	5V<br>
GND     ->	GND<br>
NSS     ->	D10<br>
MO      ->	D51<br>
SOK     ->	D52<br>
MI      ->	D50<br><br>

Buttons:<br>
UP		->	D17<br>
DOWN	->	D15<br>
MENU	->	D14<br>
OK		->	D16<br>
