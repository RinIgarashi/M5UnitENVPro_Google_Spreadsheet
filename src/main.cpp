#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <bsec2.h>

// #define DEBUG_PRINT

#ifdef DEBUG_PRINT
#define DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif

// Wi-Fi
const char *ssid = "********";
const char *password = "********";

// Google Spreadsheet
const String url = "https://script.google.com/macros/s/********/exec";

/**
 * @brief : This function checks the BSEC status, prints the respective error
 * code. Halts in case of error
 * @param[in] bsec  : Bsec2 class object
 */
void checkBsecStatus(Bsec2 bsec);

/**
 * @brief : This function is called by the BSEC library when a new output is
 * available
 * @param[in] input     : BME68X sensor data before processing
 * @param[in] outputs   : Processed BSEC BSEC output data
 * @param[in] bsec      : Instance of BSEC2 calling the callback
 */
void newDataCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec);

// Create an object of the class Bsec2
Bsec2 envSensor;

void setup() {
#ifdef DEBUG_PRINT
  Serial.begin(115200);
#endif
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  if (WiFi.begin(ssid, password) != WL_DISCONNECTED) {
    ESP.restart();
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    DEBUG_PRINTF(".");
  }

  DEBUG_PRINTF("Connected to the WiFi network!\n");

  /* Desired subscription list of BSEC2 outputs */
  bsecSensor sensorList[] = {BSEC_OUTPUT_IAQ,     BSEC_OUTPUT_RAW_TEMPERATURE,      BSEC_OUTPUT_RAW_PRESSURE, BSEC_OUTPUT_RAW_HUMIDITY,
                             BSEC_OUTPUT_RAW_GAS, BSEC_OUTPUT_STABILIZATION_STATUS, BSEC_OUTPUT_RUN_IN_STATUS};

  /* Initialize the I2C interfaces */
  Wire.begin(SDA, SCL);

#ifdef DEBUG_PRINT
  /* Valid for boards with USB-COM. Wait until the port is open */
  while (!Serial) delay(10);
#endif

  /* Initialize the library and interfaces */
  if (!envSensor.begin(BME68X_I2C_ADDR_HIGH, Wire)) {
    checkBsecStatus(envSensor);
  }

  /* Subsribe to the desired BSEC2 outputs */
  if (!envSensor.updateSubscription(sensorList, ARRAY_LEN(sensorList), BSEC_SAMPLE_RATE_LP)) {
    checkBsecStatus(envSensor);
  }

  /* Whenever new data is available call the newDataCallback function */
  envSensor.attachCallback(newDataCallback);
}

void loop() {
  if (!envSensor.run()) {
    checkBsecStatus(envSensor);
  }
}

void newDataCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec) {
  if (!outputs.nOutputs) {
    return;
  }

  String url_data = "";

  int64_t timestamp = outputs.output[0].time_stamp / INT64_C(1000000);

  url_data += "?mcu_timestamp=" + String(timestamp) + "&";

  DEBUG_PRINTF("\ntimestamp = %lld\n", outputs.output[0].time_stamp / INT64_C(1000000));
  for (uint8_t i = 0; i < outputs.nOutputs; i++) {
    const bsecData output = outputs.output[i];
    switch (output.sensor_id) {
      case BSEC_OUTPUT_IAQ:
        DEBUG_PRINTF("irq = %.1f\n", output.signal);
        DEBUG_PRINTF("iaq accuracy = %d\n", output.accuracy);
        url_data += "iaq=" + String(output.signal) + "&";
        url_data += "iaq_accuracy=" + String(output.accuracy) + "&";
        break;
      case BSEC_OUTPUT_RAW_TEMPERATURE:
        DEBUG_PRINTF("temperature = %.2f\n", output.signal);
        url_data += "temperature=" + String(output.signal) + "&";
        break;
      case BSEC_OUTPUT_RAW_PRESSURE:
        DEBUG_PRINTF("pressure = %.1f\n", output.signal);
        url_data += "pressure=" + String(output.signal) + "&";
        break;
      case BSEC_OUTPUT_RAW_HUMIDITY:
        DEBUG_PRINTF("humidity = %.2f\n", output.signal);
        url_data += "humidity=" + String(output.signal) + "&";
        break;
      case BSEC_OUTPUT_RAW_GAS:
        DEBUG_PRINTF("gas resistance = %.1f\n", output.signal);
        url_data += "gas_resistance=" + String(output.signal) + "&";
        break;
      case BSEC_OUTPUT_STABILIZATION_STATUS:
        DEBUG_PRINTF("stabilization status = %.1f\n", output.signal);
        url_data += "stabilization_status=" + String(output.signal) + "&";
        break;
      case BSEC_OUTPUT_RUN_IN_STATUS:
        DEBUG_PRINTF("run in status = %.1f\n", output.signal);
        url_data += "run_in_status=" + String(output.signal);
        break;
      default:
        break;
    }
  }

  String url_all = url + url_data;

  DEBUG_PRINTF("\nURL: %s\n", url_all.c_str());

  HTTPClient http;
  http.begin(url_all.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();

  DEBUG_PRINTF("HTTP Status Code: ");
  DEBUG_PRINTF("%d\n", httpCode);
  //---------------------------------------------------------------------
  // getting response from google sheet
  if (httpCode > 0) {
    String payload = http.getString();
    DEBUG_PRINTF("Payload: %s\n", payload.c_str());
  }
  //---------------------------------------------------------------------
  http.end();
}

void checkBsecStatus(Bsec2 bsec) {
  if (bsec.status < BSEC_OK) {
    DEBUG_PRINTF("BSEC error code : %d\n", bsec.status);
  } else if (bsec.status > BSEC_OK) {
    DEBUG_PRINTF("BSEC warning code : %d\n", bsec.status);
  }

  if (bsec.sensor.status < BME68X_OK) {
    DEBUG_PRINTF("BME68X error code : %d\n", bsec.sensor.status);
  } else if (bsec.sensor.status > BME68X_OK) {
    DEBUG_PRINTF("BME68X warning code : %d\n", bsec.sensor.status);
  }
}