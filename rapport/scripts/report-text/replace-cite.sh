#!/bin/bash

perl -pi -e 's/([.,])~(\\cite\{[a-zA-Z_]+\})/~${2}${1}/g' $@
