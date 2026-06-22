const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>

<html lang="en" data-theme="dark">
  <head>
    <meta charset="UTF-8" />

    <meta name="viewport" content="width=device-width, initial-scale=1.0" />

    <title>Labkit ESP32</title>

    <link
      href="https://fonts.googleapis.com/css2?family=Inter:wght@300;400;600&display=swap"
      rel="stylesheet"
    />

    <link
      rel="stylesheet"
      href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css"
    />

    <!-- Favicon -->
    <link rel="icon" type="image/x-icon" href="https://labkit.app/favicon.ico" />

    <script>
      var _params = new URLSearchParams(window.location.search);
      var _dev = _params.has('dev');
      var _base = _dev ? 'http://localhost:8081' : 'https://labkit.app';
      window.labkitConfig = {
        dev: _dev,
        timing: _params.get('labkit_timing') === '1'
      };
      document.write('<link rel="stylesheet" href="' + _base + '/esp/v1/labkit_ui.css" />');
    </script>


  </head>

  <body>
    <!-- Toast notifications -->

    <div id="toast" class="toast">Toast</div>

    <div class="container">
      <div class="header">
        <div class="title-container">
          <button class="home-button" onclick="window.open('https://labkit.app', '_blank')" title="Go to LabKit Home">
            <i class="fas fa-home"></i>
          </button>
          <h1>Labkit UI</h1>
          <button class="version-badge">--</button>
          <div class="controls">
            <div class="theme-switch">
              <i class="fas fa-sun"></i>
              <label class="switch">
                <input type="checkbox" id="theme-toggle" />
                <span class="slider"></span>
              </label>
              <i class="fas fa-moon"></i>
            </div>

            <div class="color-picker-container">
              <button class="color-picker-toggle">
                <i class="fas fa-palette"></i>
              </button>

              <div class="color-options">
                <button
                  class="color-option"
                  style="background-color: #4ade80"
                  data-color="#4ADE80"
                ></button>

                <button
                  class="color-option"
                  style="background-color: #60a5fa"
                  data-color="#60A5FA"
                ></button>

                <button
                  class="color-option"
                  style="background-color: #f472b6"
                  data-color="#F472B6"
                ></button>

                <button
                  class="color-option"
                  style="background-color: #fbbf24"
                  data-color="#FBBF24"
                ></button>

                <button
                  class="color-option"
                  style="background-color: #a78bfa"
                  data-color="#A78BFA"
                ></button>
              </div>
            </div>
          </div>
        </div>
      </div>

      <!-- Main content -->

      <div id="blocks" class="main-container"></div>
    </div>

    <div class="labkit-watermark">LabKit</div>


    <script src="javascript:void(0)" id="_ui_script"></script>
    <script>
      (function() {
        var s = document.createElement('script');
        s.src = _base + '/esp/v1/labkit_ui.js';
        document.getElementById('_ui_script').replaceWith(s);
      })();
    </script>

    <script
      type="module"
      src="https://unpkg.com/esp-web-tools@10/dist/web/install-button.js?module"
    ></script>

    <!-- Device information printed to console -->
    <script>
      // Device information object
      window.deviceInfo = {
        deviceName: '%DEVICE_NAME%',
        firmwareVersion: '%FIRMWARE_VERSION%',
        buildDate: '%BUILD_DATE%',
        buildTime: '%BUILD_TIME%',
        wifiSSID: '%WIFI_SSID%',
        ipAddress: '%IP_ADDRESS%',
        macAddress: '%MAC_ADDRESS%',
        uptime: '%UPTIME%',
        freeHeap: '%FREE_HEAP%',
        maxAllocHeap: '%MAX_ALLOC_HEAP%',
        features: '%DEVICE_FEATURES%'
      };

      // Print device info to console
      console.log('=== LabKit ESP32 Device Information ===');
      console.log('Device Name:', window.deviceInfo.deviceName);
      console.log('Firmware Version:', window.deviceInfo.firmwareVersion);
      console.log('Build Date:', window.deviceInfo.buildDate);
      console.log('Build Time:', window.deviceInfo.buildTime);
      console.log('WiFi SSID:', window.deviceInfo.wifiSSID);
      console.log('IP Address:', window.deviceInfo.ipAddress);
      console.log('MAC Address:', window.deviceInfo.macAddress);
      console.log('Uptime (seconds):', window.deviceInfo.uptime);
      console.log('Free Heap:', window.deviceInfo.freeHeap, 'bytes');
      console.log('Max Alloc Heap:', window.deviceInfo.maxAllocHeap, 'bytes');
      console.log('Device Features:', window.deviceInfo.features);
      console.log('=====================================');

    </script>
  </body>
</html>

)rawliteral";