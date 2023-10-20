# Tên của tệp thực thi đầu ra
TARGET_CLIENT = client
TARGET_SERVER = server

# Danh sách các tệp nguồn
SOURCES_CLIENT = udp_client.c
SOURCES_SERVER = udp_server.c

# Trình biên dịch C
CC = gcc

# Các cờ biên dịch
CFLAGS = -Wall

all: $(TARGET_CLIENT) $(TARGET_SERVER)

$(TARGET_CLIENT): $(SOURCES_CLIENT)
	$(CC) $(CFLAGS) -o $@ $<

$(TARGET_SERVER): $(SOURCES_SERVER)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET_CLIENT) $(TARGET_SERVER)
