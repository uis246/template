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

mask_override = Image.open("./mask_override.png")
autopick_override = Image.open("./pixel_override.png")

autopick_white = Image.new("RGBA", (autopick.size[0], autopick.size[1]), "white");
mask.paste(autopick_white, (0, 0), autopick_override)
mask.paste(mask_override, (0, 0), mask_override)

autopick.paste(autopick_override, (0, 0), autopick_override)
mask.save("mask.png")
autopick.save("autopick.png")