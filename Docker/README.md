# Run in docker

Simple and fast setup of ULTRAIN.IO on Docker is also available.

## Install Dependencies
 - [Docker](https://docs.docker.com) Docker 17.05 or higher is required
 - [docker-compose](https://docs.docker.com/compose/) version >= 1.10.0

## Docker Requirement
 - At least 8GB RAM (Docker -> Preferences -> Advanced -> Memory -> 8GB or above)

## Build ultrain image

```bash
git clone https://github.com/ULTRAINIO/ultrain.git --recursive
cd ultrain/Docker
docker build . -t ultrainio/ultrain
```

## Start nodultrain docker container only

```bash
docker run --name nodultrain -p 8888:8888 -p 9876:9876 -t ultrainio/ultrain nodultraind.sh arg1 arg2
```

By default, all data is persisted in a docker volume. It can be deleted if the data is outdated or corrupted:
``` bash
$ docker inspect --format '{{ range .Mounts }}{{ .Name }} {{ end }}' nodultrain
fdc265730a4f697346fa8b078c176e315b959e79365fc9cbd11f090ea0cb5cbc
$ docker volume rm fdc265730a4f697346fa8b078c176e315b959e79365fc9cbd11f090ea0cb5cbc
```

Alternately, you can directly mount host directory into the container
```bash
docker run --name nodultrain -v /path-to-data-dir:/opt/ultrainio/bin/data-dir -p 8888:8888 -p 9876:9876 -t ultrainio/ultrain nodultraind.sh arg1 arg2
```

## Get chain info

```bash
curl http://127.0.0.1:8888/v1/chain/get_info
```

## Start both nodultrain and kultraind containers

```bash
docker volume create --name=nodultrain-data-volume
docker volume create --name=kultraind-data-volume
docker-compose up -d
```

After `docker-compose up -d`, two services named `nodultraind` and `kultraind` will be started. nodultrain service would expose ports 8888 and 9876 to the host. kultraind service does not expose any port to the host, it is only accessible to clultrain when runing clultrain is running inside the kultraind container as described in "Execute clultrain commands" section.


### Execute clultrain commands

You can run the `clultrain` commands via a bash alias.

```bash
alias clultrain='docker-compose exec kultraind /opt/ultrainio/bin/clultrain -H nodultraind'
clultrain get info
clultrain get account inita
```

Upload sample exchange contract

```bash
clultrain set contract exchange contracts/exchange/exchange.wast contracts/exchange/exchange.abi
```

If you don't need kultraind afterwards, you can stop the kultraind service using

```bash
docker-compose stop kultraind
```
### Change default configuration

You can use docker compose override file to change the default configurations. For example, create an alternate config file `config2.ini` and a `docker-compose.override.yml` with the following content.

```yaml
version: "2"

services:
  nodultrain:
    volumes:
      - nodultrain-data-volume:/opt/ultrainio/bin/data-dir
      - ./config2.ini:/opt/ultrainio/bin/data-dir/config.ini
```

Then restart your docker containers as follows:

```bash
docker-compose down
docker-compose up
```

### Clear data-dir
The data volume created by docker-compose can be deleted as follows:

```bash
docker volume rm nodultrain-data-volume
docker volume rm kultraind-data-volume
```
