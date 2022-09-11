#!/bin/bash

zip -r assets.zip assets/

# build and push most recent docker image
docker build . -t dockerhub.lambertjamesd.com/lambertjamesd/n64brew2022:latest
docker push dockerhub.lambertjamesd.com/lambertjamesd/n64brew2022:latest

# upload the most up to date assets folder
sftp lambertjamesd@lambertjamesd.com:/web/n64brew2022/ <<< 'put assets.zip'

# pull the new image on the remote server
ssh lambertjamesd@lambertjamesd.com /usr/local/bin/docker pull dockerhub.lambertjamesd.com/lambertjamesd/n64brew2022:latest