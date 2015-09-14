#!/bin/bash

reset

echo "## Deleting old jobs and results ##"
rm jobs -r
rm Results -r
echo

echo "## Running simulations ##"
python RunSimHM16.6.py