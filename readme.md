# How to modify
Use gimp or another image editor with layer. Edit the mask_override.png and pixel_override.png. Those images will be then pasted over the template of the main server.

To disable a placement for the bot, just make a black mask on the selected pixels.

# How to actually update
Modifying is not enought. Change are only applied after running patch.py, then pushing the change to github

# Regular update
(template for fish shell)

``while true; git stash --all; and git pull; and echo "patching"; and  python3 patch.py; and git add autopick.png mask.png; and echo "commiting"; and git commit -m "update"; echo "pushing"; and  git push; echo "waiting..."; sleep 60; end``