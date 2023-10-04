# xsolarlogger

Lấy dữ liệu của LXP inverter từ MQTT rồi đẩy qua InfluxDB 1/2, lưu dữ liệu cuối cùng vào redis, và backup gửi vào một MQTT server khác.

# Cài đặt thư viện phụ trợ với Ubuntu

sudo apt-get install cmake libpaho-mqtt-dev libmosquitto-dev libcurl4-openssl-dev libhiredis-dev

# Biên dịch

mkdir build
cd build
cmake ..
make