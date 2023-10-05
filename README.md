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
	sudo apt-get install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu

## Tải nén sysroot của arm64 tới /tmp
	tar -xf arm64-root.tar.gz -C /tmp

## Biên dịch
	mkdir build-arm
	cd build-arm
	cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-arm64.cmake ..
	make

## Tùy chọn biên dịch

Biên dịch sử dụng thư viện Paho
	
	cmake -DPAHO 

Biên dịch hỗ trợ Redis
	
	cmake -DREDIS


## Cài đặt lên thiết bị (arm64)
Copy file lên thiết bị hoạt động (thư mục làm việc /usr/local/zsolar)
	scp xsolarlogger ubuntu@192.168.1.100:/usr/local/zsolar/bin

## Chạy ứng dụng
Khởi chạy ứng dụng với ssh

	ssh ubuntu@192.168.1.100
	/usr/local/zsolar/bin/zsolarlogger

Khởi chạy ứng dụng tự động với systemd

	sudo cp ./systemd/xsolarlogger.service /etc/systemd/system
	sudo systemctl enable xsolarlogger.service
	sudo systemctl start xsolarlogger.service

# Thiết lập hệ thống
File config lưu tại địa chỉ /usr/local/zsolar/etc, sửa đổi từ dòng lệnh bằng lệnh

	sudo nano /usr/local/zsolar/etc/config.cfg


Chú ý các tham số nguồn (src) và đích (sink)

	mqtt-src = 
	{
		host = "192.168.31.166";
		port = 1883;
		username = "";
		password = "";
		clientid = "sourcetaskcli-011";
		topic = "lxp/BA31605780";
	}

	mqtt-sink = 
	{
	    host = "103.161.39.186";
	    port = 1883;
	    username = "";
	    password = "";
	    clientid = "sinktaskcli-01";
	    topic = "lxdb/BA31605780";
	}

# Nâng cấp tiếp theo

1. Chuẩn hóa lại source - process - sink
2. Hỗ trợ nhiều loại source/sink hơn


