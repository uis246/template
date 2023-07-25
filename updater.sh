#!/bin/bash
while : do
	git stash --all
	git pull
	echo "Patching..."
	python3 patch.py
	git add autopick.png mask.png
	echo "Pushing..."
	git commit -m "Automated update."
	git push
	echo "Waiting..."
	sleep 60s
done
exit