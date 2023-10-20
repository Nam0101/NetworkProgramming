# Tên của tệp thực thi đầu ra
TARGET_CLIENT1 = client1
TARGET_CLIENT2 = client2
TARGET_SERVER = server

# Danh sách các tệp nguồn
SOURCES_CLIENT = udp_client.c
SOURCES_SERVER = udp_server.c

# Trình biên dịch C
CC = gcc

# Các cờ biên dịch
CFLAGS = -Wall

LDFLAGS_CLIENT = -lpthread
LDFLAGS_SERVER1 = -lssl
LDFLAGS_SERVER2 = -lcrypto


all: $(TARGET_CLIENT1) $(TARGET_CLIENT2) $(TARGET_SERVER)

$(TARGET_CLIENT1): $(SOURCES_CLIENT)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS_CLIENT)

$(TARGET_CLIENT2): $(SOURCES_CLIENT)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS_CLIENT)

$(TARGET_SERVER): $(SOURCES_SERVER)
	$(CC) $(CFLAGS) -o $@ $<  $(LDFLAGS_SERVER1) $(LDFLAGS_SERVER2)

clean:
	rm -f $(TARGET_CLIENT) $(TARGET_SERVER)
