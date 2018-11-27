# mysensors-wall-thermostat
MySensors OLED wall thermostat<br><br>

Wiring diagram:<br><br>

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
