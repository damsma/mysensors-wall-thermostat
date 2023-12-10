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
VCC		->	5V (red)<br>
GND		->  GND (black)<br>
DIN		->	D3 (blue)<br>
CLK     ->	D2 (yellow)<br>
CS      ->	D4 (orange)<br>
DC      ->	D5 (green)<br>
RST     ->	D6 (white)<br><br>

SHT-30:<br>
VIN     ->	3.3V (30cm Jumper Wire F/F red)<br>
GND     ->	GND (30cm Jumper Wire F/F black)<br>
SCL     ->	D21 (30cm Jumper Wire F/F blue)<br>
SAA     ->	D20 (30cm Jumper Wire F/F green)<br><br>

W5100:<br>
V+5     ->	5V (10cm Jumper Wire F/F purple)<br>
GND     ->	GND (10cm Jumper Wire F/F black)<br>
NSS     ->	D10 (10cm Jumper Wire F/F brown)<br>
MO      ->	D51 (10cm Jumper Wire F/F blue)<br>
SOK     ->	D52 (10cm Jumper Wire F/F yellow)<br>
MI      ->	D50 (10cm Jumper Wire F/F green)<br><br>

Buttons:<br>
UP		->	D17 (YTDY 10x0,5 orange)<br>
DOWN	->	D15 (YTDY 10x0,5 white)<br>
MENU	->	D14 (YTDY 10x0,5 brown)<br>
OK		->	D16 (YTDY 10x0,5 grey)<br><br>

DS18B20:<br>
1st signal  ->	D13 (YTDY 10x0,5 yellow)<br>
2nd	signal  ->	D11 (YTDY 10x0,5 green)<br>
3rd signal  ->	D9 (YTDY 10x0,5 blue)<br>
4th signal  ->	D7 (YTDY 10x0,5 purple)<br>
VCC		      ->	5V (YTDY 10x0,5 red)<br>
GND		      ->  GND (YTDY 10x0,5 black)<br>
