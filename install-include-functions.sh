#
#   This file is part of Filu.
#
#   Copyright (C) 2007, 2010, 2011, 2012, 2013 loh.tar@googlemail.com
#
#   Filu is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 2 of the License, or
#   (at your option) any later version.
#
#   Filu is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with Filu. If not, see <http:#www.gnu.org/licenses/>.
#

#
# Functions used by install-foo
#

Red="\e[1;31m"
Cyan="\e[0;36m"
noColor="\e[0m"
ColorStar="${Cyan}*${noColor}"

exitError()
{
  # Take parms:
  #   $1   Error message
  #   $2-N Error Info

  if [ ${#@} -lt 2 ]; then echo -e "${Red}*** Error ***${noColor} $1"
  else
    echo -e "${Red}*** Error ***${noColor} $1"
    echo -e "${Red}*** Info  ***${noColor} $2"

    local args=("$@")

    for (( i=2; i < ${#@}; i++))
    do
      echo -e "${Red}*** Info  ***${noColor} ${args[$i]}"
    done
  fi

  exit 1
}

checkIfRoot()
{
  if [ ! $(id -u) == 0 ]; then
    exitError "Must run as root"
  fi
}

checkPoint()
{
  # Take parms:
  #   $1 OK message
  #   $2 Next todo message

  if [ $? -ne 0 ]
  then
    exitError "Something went wrong :-("

  else
    echo -e "${ColorStar}"
    echo -e "${Cyan}*  OK:${noColor}   $1"
    echo -e "${Cyan}*  Next:${noColor} $2"
    echo -e "${ColorStar}"
    sleep 1s
  fi
}

startRolex()
{
  # Take parms:
  #   $1 Next todo message

  checkPoint "Hey! Ho! Let's Go! ...install Filu" "$1"

  # Time measuring taken from Arch Wiki. Thanks guys :-)
  # https://wiki.archlinux.org/index.php/Full_System_Backup_with_rsync
  StartTime=$(date +%s)
}

stopRolex()
{
  FinishTime=$(date +%s)
  let "Minutes = (FinishTime - StartTime) / 60"
  let "Seconds = (FinishTime - StartTime) % 60"
}

checkCommand()
{
  # Take parms:
  #   $1 command to look for

  # http://stackoverflow.com/a/4785518
  command -v $1 >/dev/null 2>&1
}

checkSystem()
{
  # Take parms:
  #   $1 command to look for
  #   $2 system name if not found

  if ! checkCommand $1
  then
    exitError "This looks not like a $2 system (found no $1)"
  fi
}

compileFilu()
{
  if [ ! -d build ]; then su $(logname) -c "mkdir build"; fi
  cd build
  su $(logname) -c "cmake .."
  checkPoint "CMake configured" "Compile programs"
  su $(logname) -c "make"
  checkPoint "Filu programs compiled" "Install programs"
  make install
  ldconfig
  cd ..
}

createCluster()
{
  if checkCommand postgresql-setup
  then
    echo "Create cluster by running 'postgresql-setup initdb'"
    postgresql-setup initdb

  else
    echo "Create cluster manually in /usr/local/pgsql/data"
    mkdir -p /usr/local/pgsql/data
    chown postgres /usr/local/pgsql/data
    su --login postgres -c "initdb -D /usr/local/pgsql/data"
  fi

  checkPoint "Cluster created" "Try again to start server"
}

startPostgreSQL()
{
  if checkCommand systemctl
  then
    echo "Run 'systemctl start postgresql'"
    if ! systemctl start postgresql >/dev/null 2>&1
    then
      checkPoint "Server not started" "Create cluster"
      createCluster
      if ! systemctl start postgresql >/dev/null 2>&1
      then
        exitError "Sorry, fail to start PostgreSQL"
      fi
    fi

  elif checkCommand service
  then
    echo "Run 'service postgresql start'"
    if ! service postgresql start >/dev/null 2>&1
    then
      checkPoint "Server not started" "Create cluster"
      createCluster
      if ! systemctl start postgresql >/dev/null 2>&1
       then
        exitError "Sorry, fail to start PostgreSQL"
      fi
    fi

  else
    exitError "Can't recognise init system" \
              "I have tested for:"          \
              "\tcommand -v systemctl"      \
              "\tcommand -v service"
  fi
}

configurePostgreSQL()
{
  checkPoint "Filu programs installed" "Check for running PostgreSQL server"

  if [ $(pgrep postgres | wc -w) -lt 1 ]
  then
    checkPoint "PostgreSQL is not running" "Try to start"
    startPostgreSQL
  fi

  checkPoint "Server is running" "Configure PostgreSQL"
  filu-cfg-postgresql
  checkPoint "PostgreSQL configured" "Stop my Rolex"
}

printFinish()
{
  stopRolex
  echo -e "${ColorStar}"
  echo -e "${ColorStar}  Total install time: $Minutes minutes, $Seconds seconds"
  echo -e "${ColorStar}"
  echo -e "${ColorStar}  Filu is ready to use :-)"
  echo -e "${ColorStar}  Take a look at your application menu below 'Office' for PerformerF, ManagerF and InspectorF"
  echo -e "${ColorStar}  You should now run as normal user 'agentf' and then, I suggest, 'agentf doc first'"
  echo -e "${ColorStar}"
}

finishInstall()
{
  compileFilu
  configurePostgreSQL
  printFinish
}
