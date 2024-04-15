#!/bin/bash

: "${GRONK_HOST:=gronk.lan}"
: "${GRONK_USER:=nycr}"

echo Deploying gronk code to $GRONK_HOST as ${GRONK_USER}.

scp -r gronk ${GRONK_USER}@${GRONK_HOST}:.
# scp -r web/* ${GRONK_USER}@${GRONK_HOST}:/var/www/gronk
# ssh ${GRONK_USER}@${GRONK_HOST} 'systemctl restart gronk gronkweb'
