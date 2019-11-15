# cd ../snark/; ./buildSnark.sh 

cd ../build; cmake ..; make

cd ../test

# cp /home/husen/zkp/zkSNARK-ult/keys/vk_demo1.dat data/
# cp /home/husen/zkp/zkSNARK-ult/input/pm_input_demo1.txt  data/
# cp /home/husen/zkp/zkSNARK-ult/proof/proof_demo1.txt   data/

pm_input=`cat data/pm_input_demo1.txt`
vk=`cat data/vk_demo1.dat`
proof=`cat data/proof_demo1.txt`
../build/./snark_test >server.log #  -a $vk';'$pm_input';'$proof
