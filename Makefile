CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -framework ApplicationServices -framework Cocoa -framework Carbon

TARGET = key_blocker
SRCS = main.c keyboard.c logger.c settings.c version.c
OBJC_SRCS = tray.m
OBJS = $(SRCS:.c=.o) $(OBJC_SRCS:.m=.o)

APP_NAME = KeyBlocker.app
APP_CONTENTS = $(APP_NAME)/Contents
APP_MACOS = $(APP_CONTENTS)/MacOS
APP_RESOURCES = $(APP_CONTENTS)/Resources

all: $(TARGET)

bundle: $(TARGET)
	mkdir -p $(APP_MACOS)
	mkdir -p $(APP_RESOURCES)
	cp $(TARGET) $(APP_MACOS)/
	cp Info.plist $(APP_CONTENTS)/
	cp assets/tray.png $(APP_RESOURCES)/
	cp AppIcon.icns $(APP_RESOURCES)/
	@echo "Bundle created: $(APP_NAME)"

dmg: bundle
	rm -f $(APP_NAME:.app=.dmg)
	mkdir -p dmg_temp
	cp -R $(APP_NAME) dmg_temp/
	ln -s /Applications dmg_temp/Applications
	hdiutil create -volname "$(APP_NAME:.app=)" -srcfolder dmg_temp -ov -format UDZO $(APP_NAME:.app=.dmg)
	rm -rf dmg_temp
	@echo "Distribution DMG created: $(APP_NAME:.app=.dmg)"

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.m
	$(CC) $(CFLAGS) -fobjc-arc -x objective-c -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
	rm -rf $(APP_NAME)
	rm -f $(APP_NAME:.app=.dmg)
	rm -rf dmg_temp

.PHONY: all clean bundle dmg
