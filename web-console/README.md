https://github.com/minimal-ui-kit/material-kit-react.git

###Found here:
https://mui.com/store/#design -> "Minimal Free – React Admin Dashboard"

###run
- `npm install`
- `npm run dev`
- `npm run build`
- `npm run build:dev`
- `npm run lint`

###docker / local / amicell
` cd web-console/server`

`docker build -t amicell .; docker run -d --name amicell -p 4000:3000 amicell; docker logs amicell;`

`docker stop amicell; docker rm amicell; docker rmi amicell;`

###docker / local / mysql
` cd web-console/server/mysql`

`docker build -t amicell/mysql .; docker run -d --name amicell -p 127.0.0.1:3307:3306 amicell/mysql;`

`docker stop amicell/mysql; docker rm amicell/mysql; docker rmi amicell/mysql;`

###docker / aws / amicell
`docker image prune -a;`

`docker buildx build --load --platform linux/arm64 -t amicell-node .;`
`docker buildx build --load --platform linux/arm64 -t amicell-node:10-dev .;`

`aws ecr get-login-password --region us-east-1 --profile dev | docker login --username AWS --password-stdin 341022751600.dkr.ecr.us-east-1.amazonaws.com`

`docker tag amicell-node:latest 341022751600.dkr.ecr.us-east-1.amazonaws.com/amicell-node:latest`
`docker tag amicell-node:10-dev 341022751600.dkr.ecr.us-east-1.amazonaws.com/amicell-node:10-dev`

`docker push 341022751600.dkr.ecr.us-east-1.amazonaws.com/amicell-node:latest`
`docker push 341022751600.dkr.ecr.us-east-1.amazonaws.com/amicell-node:10-dev`

###docker / aws / mysql
`docker image prune -a;`

`docker buildx build --load --platform linux/arm64 -t amicell-mysql .;`
`docker buildx build --load --platform linux/arm64 -t amicell-mysql:10-dev .;`

`aws ecr get-login-password --region us-east-1 --profile dev | docker login --username AWS --password-stdin 341022751600.dkr.ecr.us-east-1.amazonaws.com`

`docker tag amicell-mysql:latest 341022751600.dkr.ecr.us-east-1.amazonaws.com/amicell-mysql:latest`
`docker tag amicell-mysql:10-dev 341022751600.dkr.ecr.us-east-1.amazonaws.com/amicell-mysql:10-dev`

`docker push 341022751600.dkr.ecr.us-east-1.amazonaws.com/amicell-mysql:latest`
`docker push 341022751600.dkr.ecr.us-east-1.amazonaws.com/amicell-mysql:10-dev`

###credentials
- username `amicell`
- password `1q!QazAZ`

###Channel possible values
1. ####UBADevices table, ubaChannel column.
   - A
   - B
   - AB
2. ####RunningTests table, channel column.
    - A
    - B
2. ####InstantTestResults table, channel column.
    - A
    - B
    - A-and-B
3. ####TestRoutines table, channel column.
    - A-and-B
    - A-or-B
###May be four combinations in relation between the UBA channels and the Test channels.
1. ####If UBA have only one channel, then we can run only the single channel test only on this channel.
2. ####If UBA have two channels, then we can:
   - run the single channel tests on each channel independency. 
   - run the single channel test on one of them only. 
   - run the dual channel test on both channels.

