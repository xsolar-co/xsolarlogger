# xsolarlogger

Lấy dữ liệu của LXP inverter từ MQTT rồi đẩy qua InfluxDB 1/2, lưu dữ liệu cuối cùng vào redis, và backup gửi vào một MQTT server khác.

# Biên dịch trên Desktop (amd64)
## Cài đặt thư viện phụ trợ với Ubuntu (Biên dịch với Desktop - amd64)

    sudo apt-get install build-essential cmake libconifig-dev libcjson-dev libpaho-mqtt-dev libmosquitto-dev libcurl4-openssl-dev libhiredis-dev librdkafka-dev

## Biên dịch

    mkdir build
    cd build
    cmake ..
    make


# Biên dịch cho Rasberry PI 4, Orange Pi3, ... (arm64)
 
## Cài đặt công cụ phụ trợ
	sudo apt-get install gcc-aarch64-linux-gnu

## Biên dịch
	mkdir build
	cd build
	cmake ..
	make
