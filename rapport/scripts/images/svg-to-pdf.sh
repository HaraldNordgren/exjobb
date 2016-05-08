#!/bin/bash

inkscape $1 --export-pdf=${1%.*}.pdf
