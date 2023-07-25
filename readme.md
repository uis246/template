## How to modify
Use gimp or another image editor with layer. Edit the mask_override.png and pixel_override.png. Those images will be then pasted over the template of the main server.

To disable a placement for the bot, just make a black mask on the selected pixels.

## How to actually update
Modifying is not enought. Change are only applied after running patch.py, then pushing the change to github

## Preferred update method
### Requirements
1. Automake
2. C compiler
3. libpng with headers

### Why preferred
1. Blends mask correctly
2. Report conflicts in templates
3. Upstream mask multiplier

(template for fish shell)
``while true; git stash --all; and git pull; and echo "patching"; and make sync; echo "waiting..."; sleep 300; end``

## Alternative update method
### Requirements
1. python3
2. requests
3. pillow

(template for fish shell)
``while true; git stash --all; and git pull; and echo "patching"; and  python3 patch.py; and git add autopick.png mask.png; and echo "commiting"; and git commit -m "update"; echo "pushing"; and  git push; echo "waiting..."; sleep 300; end``

