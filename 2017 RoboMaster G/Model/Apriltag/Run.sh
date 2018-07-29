if [ $# = 1 ] && [ $1 = "-autorun" ]
then cd $(cd "$(dirname "${BASH_SOURCE}")" && pwd)
fi

echo "ubuntu" | sudo -S ./Release/Run.out
