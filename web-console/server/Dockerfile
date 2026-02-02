FROM node:22-alpine

ENV DB_CONFIGURATION_HOST=dev-amicell-mysql.cluster-cza84fhjccav.us-east-1.rds.amazonaws.com

ENV DB_CONFIGURATION_PASSWORD=Wc316ScIVo1s

RUN mkdir -p /usr/src/app

WORKDIR /usr/src/app

COPY . ./

RUN npm install

CMD [ "node", "./bin/www" ]
