#!/bin/bash
for file in ./* ;do unzip "$file" -d  "$(awk -F'[/.]' '{print $3}' <<< $file)" ; done

