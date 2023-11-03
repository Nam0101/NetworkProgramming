# Tên của tệp thực thi đầu ra
TARGET_CLIENT = client
TARGET_SERVER = server

# Danh sách các tệp nguồn
SOURCES_CLIENT = tcp_client.c
SOURCES_SERVER = tcp_server.c

# Trình biên dịch C
CC = gcc

# Các cờ biên dịch
CFLAGS = -Wall
LDFLAGS_SERVER1 = -lssl
LDFLAGS_SERVER2 = -lcrypto

all: $(TARGET_CLIENT) $(TARGET_SERVER)

$(TARGET_CLIENT): $(SOURCES_CLIENT)
	$(CC) $(CFLAGS) -o $@ $< 


$(TARGET_SERVER): $(SOURCES_SERVER)
	$(CC) $(CFLAGS) -o $@ $<  $(LDFLAGS_SERVER1) $(LDFLAGS_SERVER2)

clean:
	rm -f $(TARGET_CLIENT) $(TARGET_SERVER)
