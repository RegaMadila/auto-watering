
#define BLYNK_TEMPLATE_ID "TMPL6apqrAj3C"
#define BLYNK_TEMPLATE_NAME "Temp Humid Monitoring"
#define BLYNK_AUTH_TOKEN "U1gxCa3g7gYdmfd0fM7tz77d0Qcnm3Og"

#define BLYNK_PRINT Serial
#include <Fuzzy.h>
#include <DHT.h>
#include <BlynkSimpleEsp32.h>
#include <WiFi.h>

#define DHTPIN 5     // Pin tempat Anda menghubungkan output DHT11
#define DHTTYPE DHT11 // Tipe sensor DHT yang digunakan
#define SOIL_MOISTURE_PIN 33 // Pin tempat Anda menghubungkan sensor kelembaban tanah
#define LED_PIN 2
#define RELAY1 13

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Astroworld";  // type your wifi name
char pass[] = "jumpman23";  // type your wifi password

BlynkTimer timer;

DHT dht(DHTPIN, DHTTYPE);

const int RELAY_PIN  = 13;

// Fuzzy
Fuzzy *fuzzy = new Fuzzy();

// FuzzyInput
FuzzySet *cool = new FuzzySet(0, 0, 5, 20);
FuzzySet *normal = new FuzzySet(15, 25, 25, 30);
FuzzySet *hot = new FuzzySet(30, 35, 40, 40 );

// FuzzyInput for Soil Moisture
FuzzySet *dry = new FuzzySet(0, 20, 20, 40);
FuzzySet *moist = new FuzzySet(30, 50, 50, 70);
FuzzySet *wet = new FuzzySet(60, 80, 100, 100);

// FuzzyOutput
FuzzySet *shortDuration = new FuzzySet(0, 0, 0, 2);
FuzzySet *mediumDuration = new FuzzySet(2, 3, 3, 4);
FuzzySet *longDuration = new FuzzySet(3, 5, 5, 5);

void activateRelay(int duration) {
  digitalWrite(RELAY_PIN, LOW);  // Turn on relay
  delay(duration * 1000);          // Wait for the specified duration in milliseconds
  digitalWrite(RELAY_PIN, HIGH);   // Turn off relay
}

void sendSensor()
{
  int soil;
  float temperatureInput = dht.readTemperature();
  float humidityInput = dht.readHumidity();
  int soilHumidityInput = analogRead(SOIL_MOISTURE_PIN);
  soilHumidityInput = map(analogRead(SOIL_MOISTURE_PIN), 0, 4095, 100, 0);

  Serial.println("\n\n\nEntrance: ");
  Serial.print("\t\t\tTemperature: ");
  Serial.println(temperatureInput);
  Serial.print("\t\t\tSoil Humidity: ");
  Serial.println(soilHumidityInput);
  
  fuzzy->setInput(1, temperatureInput);
  fuzzy->setInput(2, soilHumidityInput);

  fuzzy->fuzzify();

  Serial.println("Input: ");
  Serial.print("\tTemperature: Cool-> ");
  Serial.print(cool->getPertinence());
  Serial.print(", Normal-> ");
  Serial.print(normal->getPertinence());
  Serial.print(", Hot-> ");
  Serial.println(hot->getPertinence());

  Serial.print("\tSoil Humidity: Dry-> ");
  Serial.print(dry->getPertinence());
  Serial.print(",  Moist-> ");
  Serial.print(moist->getPertinence());
  Serial.print(",  Wet-> ");
  Serial.println(wet->getPertinence());

  float wateringDurationOutput = fuzzy->defuzzify(1);

  Serial.println("Output: ");
  Serial.print("\tWatering Duration: Short-> ");
  Serial.print(shortDuration->getPertinence());
  Serial.print(", Medium-> ");
  Serial.print(mediumDuration->getPertinence());
  Serial.print(", Long-> ");
  Serial.println(longDuration->getPertinence());

  Serial.println("Result: ");
  Serial.print("\t\t\tWatering Duration: ");
  Serial.println(wateringDurationOutput);

  Blynk.virtualWrite(V0, temperatureInput);
  Blynk.virtualWrite(V1, humidityInput);
  Blynk.virtualWrite(V3, soilHumidityInput);
  Blynk.virtualWrite(V4, wateringDurationOutput);

  activateRelay(static_cast<int>(wateringDurationOutput));
}

void setup()
{
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  dht.begin();
  pinMode(RELAY_PIN, OUTPUT);

  // FuzzyInput
  FuzzyInput *temperature = new FuzzyInput(1);

  temperature->addFuzzySet(cool);
  temperature->addFuzzySet(normal);
  temperature->addFuzzySet(hot);
  fuzzy->addFuzzyInput(temperature);

  // FuzzyInput
  FuzzyInput *soilHumidity = new FuzzyInput(2);

  soilHumidity->addFuzzySet(dry);
  soilHumidity->addFuzzySet(moist);
  soilHumidity->addFuzzySet(wet);
  fuzzy->addFuzzyInput(soilHumidity);

  // FuzzyOutput
  FuzzyOutput *wateringDuration = new FuzzyOutput(1);

  wateringDuration->addFuzzySet(shortDuration);
  wateringDuration->addFuzzySet(mediumDuration);
  wateringDuration->addFuzzySet(longDuration);
  fuzzy->addFuzzyOutput(wateringDuration);

  // Building FuzzyRule
  FuzzyRuleAntecedent *coolAndDry = new FuzzyRuleAntecedent();
  coolAndDry->joinWithAND(cool, dry);
  FuzzyRuleAntecedent *normalAndDry = new FuzzyRuleAntecedent();
  normalAndDry->joinWithAND(normal, dry);
  FuzzyRuleAntecedent *hotAndDry = new FuzzyRuleAntecedent();
  hotAndDry->joinWithAND(hot, dry);
  FuzzyRuleAntecedent *coolAndMoist = new FuzzyRuleAntecedent();
  coolAndMoist->joinWithAND(cool, moist);
  FuzzyRuleAntecedent *normalAndMoist = new FuzzyRuleAntecedent();
  normalAndMoist->joinWithAND(normal, moist);
  FuzzyRuleAntecedent *hotAndMoist = new FuzzyRuleAntecedent();
  hotAndMoist->joinWithAND(hot, moist);
  FuzzyRuleAntecedent *coolAndWet = new FuzzyRuleAntecedent();
  coolAndWet->joinWithAND(cool, wet);
  FuzzyRuleAntecedent *normalAndWet = new FuzzyRuleAntecedent();
  normalAndWet->joinWithAND(normal, wet);
  FuzzyRuleAntecedent *hotAndWet = new FuzzyRuleAntecedent();
  hotAndWet->joinWithAND(hot, wet);

  FuzzyRuleConsequent *thenLongDuration = new FuzzyRuleConsequent();
  thenLongDuration->addOutput(longDuration);
  FuzzyRuleConsequent *thenMediumDuration = new FuzzyRuleConsequent();
  thenMediumDuration->addOutput(mediumDuration);
  FuzzyRuleConsequent *thenShortDuration = new FuzzyRuleConsequent();
  thenShortDuration->addOutput(shortDuration);

  FuzzyRule *fuzzyRule1 = new FuzzyRule(1, coolAndDry, thenMediumDuration);
  FuzzyRule *fuzzyRule2 = new FuzzyRule(2, normalAndDry, thenMediumDuration);
  FuzzyRule *fuzzyRule3 = new FuzzyRule(3, hotAndDry, thenLongDuration);
  FuzzyRule *fuzzyRule4 = new FuzzyRule(4, coolAndMoist, thenShortDuration);
  FuzzyRule *fuzzyRule5 = new FuzzyRule(5, normalAndMoist, thenShortDuration);
  FuzzyRule *fuzzyRule6 = new FuzzyRule(6, hotAndMoist, thenLongDuration);
  FuzzyRule *fuzzyRule7 = new FuzzyRule(7, coolAndWet, thenShortDuration);
  FuzzyRule *fuzzyRule8 = new FuzzyRule(8, normalAndWet, thenShortDuration);
  FuzzyRule *fuzzyRule9 = new FuzzyRule(9, hotAndWet, thenShortDuration);

  fuzzy->addFuzzyRule(fuzzyRule1);
  fuzzy->addFuzzyRule(fuzzyRule2);
  fuzzy->addFuzzyRule(fuzzyRule3);
  fuzzy->addFuzzyRule(fuzzyRule4);
  fuzzy->addFuzzyRule(fuzzyRule5);
  fuzzy->addFuzzyRule(fuzzyRule6);
  fuzzy->addFuzzyRule(fuzzyRule7);
  fuzzy->addFuzzyRule(fuzzyRule8);
  fuzzy->addFuzzyRule(fuzzyRule9);

  timer.setInterval(6000L, sendSensor);  // Adjust the interval as needed
}

void loop()
{
  Blynk.run();
  timer.run();
}

BLYNK_WRITE(V5) // Mendeteksi perubahan pada widget tombol Blynk
{
    int value = param.asInt(); // Membaca nilai tombol (0 atau 1)
    int RELAY1_STATE = param.asInt();
    digitalWrite(LED_PIN, value);
    digitalWrite(RELAY1, RELAY1_STATE); // Mengontrol LED berdasarkan nilai tombol
}
