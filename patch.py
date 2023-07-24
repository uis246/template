import requests
import io
from PIL import Image

def download_image(url):
    r = requests.get(url)
    if r.status_code != requests.codes.ok:
        assert False, 'Status code error: {}.'.format(r.status_code)

    return Image.open(io.BytesIO(r.content))

autopick = download_image("https://ponyplace.z19.web.core.windows.net/mlp/autopick.png").copy()
mask = download_image("https://ponyplace.z19.web.core.windows.net/mlp/mask.png").copy()

mask_override = Image.open("./mask_override_ponk.png")
autopick_override = Image.open("./pixel_override_ponk.png")

autopick_white = Image.new("RGBA", (autopick.size[0], autopick.size[1]), "white")
mask.paste(autopick_white, (0, 0), autopick_override)
mask.paste(mask_override, (0, 0), mask_override)

autopick.paste(autopick_override, (0, 0), autopick_override)

# make autopick transparent for black pixel in mask
mask_override_full_black = mask_override.copy()
mask_override_full_black_load = mask_override_full_black.load()
for x in range(mask_override_full_black.size[0]):
    for y in range(mask_override_full_black.size[1]):
        if mask_override_full_black_load[x, y] != (0, 0, 0, 255):
            mask_override_full_black_load[x, y] = (0, 0, 0, 0)
autopick_alpha = Image.new("RGBA", (autopick.size[0], autopick.size[1]), (0, 0, 0, 0))
autopick.paste(autopick_alpha, (0, 0), mask_override_full_black)

mask.save("mask.png")
autopick.save("autopick.png")
