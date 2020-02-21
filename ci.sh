#!/bin/bash

rm co2/secrets.h || true
cp co2/secrets.sample.h co2/secrets.h
platformio run
rm co2/secrets.h
