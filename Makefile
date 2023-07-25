CC ?= gcc

bin/merger: src/main.c
	@mkdir -p bin
	$(CC) -Os -s src/main.c -o bin/merger -lpng

all: bin/merger

template: bin/merger
	rm -f upstream.png upstream_mask.png
	wget https://ponyplace.z19.web.core.windows.net/mlp/autopick.png -O upstream.png
	wget https://ponyplace.z19.web.core.windows.net/mlp/mask.png -O upstream_mask.png
	bin/merger pixel_override.png mask_override.png 0.98
	rm upstream.png upstream_mask.png

sync: template
	git add autopick.png mask.png
	git commit -m "Update template from upstream"
	git push

do: template
	git add autopick.png mask.png
	git commit
	git push

