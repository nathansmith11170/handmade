#!/bin/bash

cd code
bear --output ../compile_commands.json -- make
cd ..
