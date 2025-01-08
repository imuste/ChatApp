#! /bin/sh

./make.sh
./serverDriver 9099 > server_Out.txt &
serverPID=$!


#CLIENT 1 TEST
./client1 > client1_Out.txt
if diff client1_Out.txt client1_GT.txt > /dev/null; then
        echo "Client1 output matches"
else
        echo "CLIENT1 OUTPUT DIFFER"
        diff client1_Out.txt client1_GT.txt
fi


#CLIENT 2 TEST
# client writes HELLO twice, client already present ERROR
./client2 > client2_Out.txt
if diff client2_Out.txt client2_GT.txt > /dev/null; then
        echo "Client2 output matches"
else
        echo "CLIENT2 OUTPUT DIFFER"
        diff client2_Out.txt client2_GT.txt
fi


#CLIENT 3 TEST
./client3 > client3_Out.txt
if diff client3_Out.txt client3_GT.txt > /dev/null; then
        echo "Client3 output matches"
else
        echo "CLIENT3 OUTPUT DIFFER"
        diff client3_Out.txt client3_GT.txt
fi


#CLIENT 4 TEST
# client writes two messages at once
./client4 > client4_Out.txt
if diff client4_Out.txt client4_GT.txt > /dev/null; then
        echo "Client4 output matches"
else
        echo "CLIENT4 OUTPUT DIFFER"
        diff client4_Out.txt client4_GT.txt
fi


#CLIENT 5/6 TEST
# regular - no edge case chat between two clients
./client5 > client5_Out.txt & 
client5PID=$!
sleep 1
./client6 > client6_Out.txt &
client6PID=$!

# Wait for both clients to finish
wait $client5PID
wait $client6PID

# Check output for client5
if diff client5_Out.txt client5_GT.txt > /dev/null; then
    echo "Client5 output matches"
else
    echo "CLIENT5 OUTPUT DIFFER"
    diff client5_Out.txt client5_GT.txt
fi

# Check output for client6
if diff client6_Out.txt client6_GT.txt > /dev/null; then
    echo "Client6 output matches"
else
    echo "CLIENT6 OUTPUT DIFFER"
    diff client6_Out.txt client6_GT.txt
fi


#CLIENT 7 TEST
# write CLIENT LIST message split up with sleep in between
./client7 > client7_Out.txt
if diff client7_Out.txt client7_GT.txt > /dev/null; then
        echo "Client7 output matches"
else
        echo "CLIENT7 OUTPUT DIFFER"
        diff client7_Out.txt client7_GT.txt
fi


#CLIENT 8 TEST
# write HELLO message split up with sleep in between
# write EXIT message split up with sleep in between
./client8 > client8_Out.txt
if diff client8_Out.txt client8_GT.txt > /dev/null; then
        echo "Client8 output matches"
else
        echo "CLIENT8 OUTPUT DIFFER"
        diff client8_Out.txt client8_GT.txt
fi


#CLIENT 9/10 TEST
# write CHAT message header split up with sleep in between
# write CHAT message body split up with sleep in between
./client9 > client9_Out.txt & 
client9PID=$!
sleep 1
./client10 > client10_Out.txt &
client10PID=$!

# Wait for both clients to finish
wait $client9PID
wait $client10PID

# Check output for client9
if diff client9_Out.txt client9_GT.txt > /dev/null; then
    echo "Client9 output matches"
else
    echo "CLIENT9 OUTPUT DIFFER"
    diff client9_Out.txt client9_GT.txt
fi

# Check output for client10
if diff client10_Out.txt client10_GT.txt > /dev/null; then
    echo "Client10 output matches"
else
    echo "CLIENT10 OUTPUT DIFFER"
    diff client10_Out.txt client10_GT.txt
fi


#CLIENT 11 TEST
# client enters with different message than HELLO
./client11 > client11_Out.txt
if diff client11_Out.txt client11_GT.txt > /dev/null; then
        echo "Client11 output matches"
else
        echo "CLIENT11 OUTPUT DIFFER"
        diff client11_Out.txt client11_GT.txt
fi


#CLIENT 12 TEST
# client leaves without EXIT
./client12 > client12_Out.txt
if diff client12_Out.txt client12_GT.txt > /dev/null; then
        echo "Client12 output matches"
else
        echo "CLIENT12 OUTPUT DIFFER"
        diff client12_Out.txt client12_GT.txt
fi


#CLIENT 13/14 TEST
# client 1 sends partial message to server, client 2 sends CHAT to client 1, 
# client 1 reads CHAT, client 1 finishes partial message to server
./client13 > client13_Out.txt & 
client13PID=$!
sleep 1
./client14 > client14_Out.txt &
client14PID=$!

# Wait for both clients to finish
wait $client13PID
wait $client14PID

# Check output for client13
if diff client13_Out.txt client13_GT.txt > /dev/null; then
    echo "Client13 output matches"
else
    echo "CLIENT13 OUTPUT DIFFER"
    diff client13_Out.txt client13_GT.txt
fi

# Check output for client14
if diff client14_Out.txt client14_GT.txt > /dev/null; then
    echo "Client14 output matches"
else
    echo "CLIENT14 OUTPUT DIFFER"
    diff client14_Out.txt client14_GT.txt
fi



#CLIENT 15/16 TEST
# one client has ID "client15" and other client has ID "Client15", so should
# be treated as different clients
./client15 > client15_Out.txt & 
client15PID=$!
sleep 1
./client16 > client16_Out.txt &
client16PID=$!

# Wait for both clients to finish
wait $client15PID
wait $client16PID

# Check output for client15
if diff client15_Out.txt client15_GT.txt > /dev/null; then
    echo "Client15 output matches"
else
    echo "CLIENT15 OUTPUT DIFFER"
    diff client15_Out.txt client15_GT.txt
fi

# Check output for client16
if diff client16_Out.txt client16_GT.txt > /dev/null; then
    echo "Client16 output matches"
else
    echo "CLIENT16 OUTPUT DIFFER"
    diff client16_Out.txt client16_GT.txt
fi


#CLIENT 17/18 TEST
# client17 sends partial HELLO, client 18 sends HELLO and shouldn't get 
# client17 in its CLIENT_LIST
./client17 > client17_Out.txt & 
client17PID=$!
sleep 1
./client18 > client18_Out.txt &
client18PID=$!

# Wait for both clients to finish
wait $client17PID
wait $client18PID

# Check output for client17
if diff client17_Out.txt client17_GT.txt > /dev/null; then
    echo "Client17 output matches"
else
    echo "CLIENT17 OUTPUT DIFFER"
    diff client17_Out.txt client17_GT.txt
fi

# Check output for client18
if diff client18_Out.txt client18_GT.txt > /dev/null; then
    echo "Client18 output matches"
else
    echo "CLIENT18 OUTPUT DIFFER"
    diff client18_Out.txt client18_GT.txt
fi


#CLIENT 19/20 TEST
# client19 writes HELLO, CLIENT_LIST_REQ, and 2 x CHAT at once to the server
# client20 reads both CHAT messages
./client20 > client20_Out.txt & 
client20PID=$!
sleep 1
./client19 > client19_Out.txt &
client19PID=$!

# Wait for both clients to finish
wait $client19PID
wait $client20PID

# Check output for client19
if diff client19_Out.txt client19_GT.txt > /dev/null; then
    echo "Client19 output matches"
else
    echo "CLIENT19 OUTPUT DIFFER"
    diff client19_Out.txt client19_GT.txt
fi

# Check output for client20
if diff client20_Out.txt client20_GT.txt > /dev/null; then
    echo "Client20 output matches"
else
    echo "CLIENT20 OUTPUT DIFFER"
    diff client20_Out.txt client20_GT.txt
fi



#CLIENT 21/22/23/24 TEST
# many different clients connect to the server and send messages back and forth
./client21 > client21_Out.txt & 
client21PID=$!
sleep 1
./client22 > client22_Out.txt &
client22PID=$!
sleep 1
./client23 > client23_Out.txt &
client23PID=$!
sleep 1
./client24 > client24_Out.txt &
client24PID=$!

# Wait for clients to finish
wait $client21PID
wait $client22PID
wait $client23PID
wait $client24PID

# Check output for client21
if diff client21_Out.txt client21_GT.txt > /dev/null; then
    echo "Client21 output matches"
else
    echo "CLIENT21 OUTPUT DIFFER"
    diff client21_Out.txt client21_GT.txt
fi

# Check output for client22
if diff client22_Out.txt client22_GT.txt > /dev/null; then
    echo "Client22 output matches"
else
    echo "CLIENT22 OUTPUT DIFFER"
    diff client22_Out.txt client22_GT.txt
fi

# Check output for client23
if diff client23_Out.txt client23_GT.txt > /dev/null; then
    echo "Client23 output matches"
else
    echo "CLIENT23 OUTPUT DIFFER"
    diff client23_Out.txt client23_GT.txt
fi

# Check output for client24
if diff client24_Out.txt client24_GT.txt > /dev/null; then
    echo "Client24 output matches"
else
    echo "CLIENT24 OUTPUT DIFFER"
    diff client24_Out.txt client24_GT.txt
fi



#CLIENT 25/26 TEST
# client25 writes a 400 byte message to client 26
./client25 > client25_Out.txt & 
client25PID=$!
sleep 1
./client26 > client26_Out.txt &
client26PID=$!

# Wait for both clients to finish
wait $client25PID
wait $client26PID

# Check output for client25
if diff client25_Out.txt client25_GT.txt > /dev/null; then
    echo "Client25 output matches"
else
    echo "CLIENT25 OUTPUT DIFFER"
    diff client25_Out.txt client25_GT.txt
fi

# Check output for client26
if diff client26_Out.txt client26_GT.txt > /dev/null; then
    echo "Client26 output matches"
else
    echo "CLIENT26 OUTPUT DIFFER"
    diff client26_Out.txt client26_GT.txt
fi


#CLIENT 27 TEST
# client27 writes CHAT to non-existent client
./client27 > client27_Out.txt
if diff client27_Out.txt client27_GT.txt > /dev/null; then
        echo "Client27 output matches"
else
        echo "CLIENT27 OUTPUT DIFFER"
        diff client27_Out.txt client27_GT.txt
fi


#CLIENT 28 TEST
# client28 writes part of CLIENT_REQ and waits longer than 60 seconds
./client28 > client28_Out.txt
if diff client28_Out.txt client28_GT.txt > /dev/null; then
        echo "Client28 output matches"
else
        echo "CLIENT28 OUTPUT DIFFER"
        diff client28_Out.txt client28_GT.txt
fi



#CLIENT 29/30 TEST
# client30 tries to write CHAT to client29 before it's connected, so 
# retransmitts once client29 is connected
./client29 > client29_Out.txt & 
client29PID=$!
sleep 1
./client30 > client30_Out.txt &
client30PID=$!

# Wait for both clients to finish
wait $client29PID
wait $client30PID

# Check output for client29
if diff client29_Out.txt client29_GT.txt > /dev/null; then
    echo "Client29 output matches"
else
    echo "CLIENT29 OUTPUT DIFFER"
    diff client29_Out.txt client29_GT.txt
fi

# Check output for client30
if diff client30_Out.txt client30_GT.txt > /dev/null; then
    echo "Client30 output matches"
else
    echo "CLIENT30 OUTPUT DIFFER"
    diff client30_Out.txt client30_GT.txt
fi


#CLIENT 31/32 TEST
# client31 connects and immediately leaves, then client32 tries to send a CHAT
# to client31 and gets error
./client31 > client31_Out.txt & 
client31PID=$!
sleep 1
./client32 > client32_Out.txt &
client32PID=$!

# Wait for both clients to finish
wait $client31PID
wait $client32PID

# Check output for client31
if diff client31_Out.txt client31_GT.txt > /dev/null; then
    echo "Client31 output matches"
else
    echo "CLIENT31 OUTPUT DIFFER"
    diff client31_Out.txt client31_GT.txt
fi

# Check output for client32
if diff client32_Out.txt client32_GT.txt > /dev/null; then
    echo "Client32 output matches"
else
    echo "CLIENT32 OUTPUT DIFFER"
    diff client32_Out.txt client32_GT.txt
fi


#CLIENT 33/Z1 TEST
# uses example client "assign-2client" to test reading and writing 
./client33 > client33_Out.txt & 
client33PID=$!
sleep 1
./assign-2client 10.4.2.18 9099 clientZ1 << EOF > clientZ1_Out.txt &
3
client33
EOF
clientZ1PID=$!

# Wait for both clients to finish
wait $client33PID
wait $clientZ1PID

# Check output for client33
if diff client33_Out.txt client33_GT.txt > /dev/null; then
    echo "Client33 output matches"
else
    echo "CLIENT33 OUTPUT DIFFER"
    diff client33_Out.txt client33_GT.txt
fi

# Check output for clientZ1
if diff clientZ1_Out.txt clientZ1_GT.txt > /dev/null; then
    echo "ClientZ1 output matches"
else
    echo "CLIENTZ1 OUTPUT DIFFER"
    diff clientZ1_Out.txt clientZ1_GT.txt
fi


#CLIENT 34/Z2 TEST
# uses example client "assign-2client" to test reading and writing 
./assign-2client 10.4.2.18 9099 clientZ2 << EOF > clientZ2_Out.txt &
2
EOF
clientZ2PID=$!
sleep 1
./client34 > client34_Out.txt & 
client34PID=$!

# Wait for both clients to finish
wait $client34PID
wait $clientZ2PID

# Check output for client34
if diff client34_Out.txt client34_GT.txt > /dev/null; then
    echo "Client34 output matches"
else
    echo "CLIENT34 OUTPUT DIFFER"
    diff client34_Out.txt client34_GT.txt
fi

# Check output for clientZ2
if diff clientZ2_Out.txt clientZ2_GT.txt > /dev/null; then
    echo "ClientZ2 output matches"
else
    echo "CLIENTZ2 OUTPUT DIFFER"
    diff clientZ2_Out.txt clientZ2_GT.txt
fi


#CLIENT 35/36 TEST
# client35 connects, sends CHAT in 3 parts with sleep in betwee, client36 times
# out, when client35 CHAT is complete, ERROR_CANNOT_DELIVER is sent
# to client31 and gets error
./client35 > client35_Out.txt & 
client35PID=$!
sleep 1
./client36 > client36_Out.txt &
client36PID=$!

# Wait for both clients to finish
wait $client35PID
wait $client36PID

# Check output for client35
if diff client35_Out.txt client35_GT.txt > /dev/null; then
    echo "Client35 output matches"
else
    echo "CLIENT35 OUTPUT DIFFER"
    diff client35_Out.txt client35_GT.txt
fi

# Check output for client36
if diff client36_Out.txt client36_GT.txt > /dev/null; then
    echo "Client36 output matches"
else
    echo "CLIENT36 OUTPUT DIFFER"
    diff client36_Out.txt client36_GT.txt
fi


#CLIENT 37 TEST
# client37 writes part of HELLO and waits longer than 60 seconds
./client37 > client37_Out.txt
if diff client37_Out.txt client37_GT.txt > /dev/null; then
        echo "Client37 output matches"
else
        echo "CLIENT37 OUTPUT DIFFER"
        diff client37_Out.txt client37_GT.txt
fi


#CLIENT 38/39 TEST
# client38 writes HELLO and waits 35 seconds for other clients to test with this
# client39 writes HELLO and sends CLIENT_LIST_REQ with wrong ID, so gets disconnected
./client38 > client38_Out.txt & 
client38PID=$!
sleep 1
./client39 > client39_Out.txt &
client39PID=$!

# Wait for both clients to finish
wait $client38PID
wait $client39PID

# Check output for client38
if diff client38_Out.txt client38_GT.txt > /dev/null; then
        echo "Client38 output matches"
else
        echo "CLIENT38 OUTPUT DIFFER"
        diff client38_Out.txt client38_GT.txt
fi

# Check output for client39
if diff client39_Out.txt client39_GT.txt > /dev/null; then
        echo "Client39 output matches"
else
        echo "CLIENT39 OUTPUT DIFFER"
        diff client39_Out.txt client39_GT.txt
fi


#CLIENT 40 TEST
# client40 writes HELLO and CHAT to itself, so should get disconnected
./client40 > client40_Out.txt
if diff client40_Out.txt client40_GT.txt > /dev/null; then
        echo "Client40 output matches"
else
        echo "CLIENT40 OUTPUT DIFFER"
        diff client40_Out.txt client40_GT.txt
fi


#CLIENT 38/41 TEST
# client41 sends HELLO with client38 ID, so get's CLIENT_ALREADY_EXISTS error
./client38 > client38_Out.txt & 
client38PID=$!
sleep 1
./client41 > client41_Out.txt &
client41PID=$!

# Wait for both clients to finish
wait $client38PID
wait $client41PID

# Check output for client41
if diff client41_Out.txt client41_GT.txt > /dev/null; then
        echo "Client41 output matches"
else
        echo "CLIENT41 OUTPUT DIFFER"
        diff client41_Out.txt client41_GT.txt
fi


#CLIENT 38/42 TEST
# client42 sends HELLO with client42 ID, then sends HELLO with client38 ID,
# so get's CLIENT_ALREADY_EXISTS error
./client38 > client38_Out.txt & 
client38PID=$!
sleep 1
./client42 > client42_Out.txt &
client42PID=$!

# Wait for both clients to finish
wait $client38PID
wait $client42PID

# Check output for client41
if diff client42_Out.txt client42_GT.txt > /dev/null; then
        echo "Client42 output matches"
else
        echo "CLIENT42 OUTPUT DIFFER"
        diff client42_Out.txt client42_GT.txt
fi


#CLIENT 38/43 TEST
# client43 sends HELLO with client43 ID, then sends HELLO with client44 ID,
# so gets disconnected
./client38 > client38_Out.txt & 
client38PID=$!
sleep 1
./client43 > client43_Out.txt &
client43PID=$!

# Wait for both clients to finish
wait $client38PID
wait $client43PID

# Check output for client41
if diff client43_Out.txt client43_GT.txt > /dev/null; then
        echo "Client43 output matches"
else
        echo "CLIENT43 OUTPUT DIFFER"
        diff client43_Out.txt client43_GT.txt
fi


#CLIENT 38/44 TEST
# client43 sends HELLO with client43 ID, then sends HELLO with client44 ID,
# so gets disconnected
./client38 > client38_Out.txt & 
client38PID=$!
sleep 1
./client44 > client44_Out.txt &
client44PID=$!

# Wait for both clients to finish
wait $client38PID
wait $client44PID

# Check output for client44
if diff client44_Out.txt client44_GT.txt > /dev/null; then
        echo "Client44 output matches"
else
        echo "CLIENT44 OUTPUT DIFFER"
        diff client44_Out.txt client44_GT.txt
fi



#CLIENT 45/46 TEST
# client45 writes HELLO, CLIENT_LIST_REQ, and 2 x CHAT of which 2nd CHAT is 
# invalid, at once to the server. client20 reads first CHAT message
./client46 > client46_Out.txt & 
client46PID=$!
sleep 2
./client45 > client45_Out.txt &
client45PID=$!

# Wait for both clients to finish
wait $client45PID
wait $client46PID

# Check output for client45
if diff client45_Out.txt client45_GT.txt > /dev/null; then
    echo "Client45 output matches"
else
    echo "CLIENT45 OUTPUT DIFFER"
    diff client45_Out.txt client45_GT.txt
fi

# Check output for client46
if diff client46_Out.txt client46_GT.txt > /dev/null; then
    echo "Client46 output matches"
else
    echo "CLIENT46 OUTPUT DIFFER"
    diff client46_Out.txt client46_GT.txt
fi


#CLIENT 47 TEST
# client47 writes valid message, invalid message, and valid message
./client47 > client47_Out.txt
if diff client47_Out.txt client47_GT.txt > /dev/null; then
        echo "Client47 output matches"
else
        echo "CLIENT47 OUTPUT DIFFER"
        diff client47_Out.txt client47_GT.txt
fi


#CLIENT 38/48 TEST
# client48 sends CHAT message with length 0, and real length 30, so times out 
./client38 > client38_Out.txt & 
client38PID=$!
sleep 1
./client48 > client48_Out.txt &
client48PID=$!

# Wait for both clients to finish
wait $client38PID
wait $client48PID

# Check output for client48
if diff client48_Out.txt client48_GT.txt > /dev/null; then
        echo "Client48 output matches"
else
        echo "CLIENT48 OUTPUT DIFFER"
        diff client48_Out.txt client48_GT.txt
fi


#CLIENT 38/49 TEST
# client49 sends CHAT message with length 0, and real length 55, so gets 
# removed b/c invalid header
./client38 > client38_Out.txt & 
client38PID=$!
sleep 1
./client49 > client49_Out.txt &
client49PID=$!

# Wait for both clients to finish
wait $client38PID
wait $client49PID

# Check output for client49
if diff client49_Out.txt client49_GT.txt > /dev/null; then
        echo "Client49 output matches"
else
        echo "CLIENT49 OUTPUT DIFFER"
        diff client49_Out.txt client49_GT.txt
fi



#CLIENT 50/51 TEST
# clients write empty CHAT messages
./client50 > client50_Out.txt & 
client50PID=$!
./client51 > client51_Out.txt &
client51PID=$!

# Wait for both clients to finish
wait $client50PID
wait $client51PID

# Check output for client50
if diff client50_Out.txt client50_GT.txt > /dev/null; then
    echo "Client50 output matches"
else
    echo "CLIENT50 OUTPUT DIFFER"
    diff client50_Out.txt client50_GT.txt
fi

# Check output for client51
if diff client51_Out.txt client51_GT.txt > /dev/null; then
    echo "Client51 output matches"
else
    echo "CLIENT51 OUTPUT DIFFER"
    diff client51_Out.txt client51_GT.txt
fi



#############################################################################
#                              HEADER TESTING
#############################################################################

#CLIENT 52 TEST
./client52 > client52_Out.txt
if diff client52_Out.txt clientH1_GT.txt > /dev/null; then
        echo "Client52 output matches"
else
        echo "CLIENT52 OUTPUT DIFFER"
        diff client52_Out.txt clientH1_GT.txt
fi

#CLIENT 53 TEST
./client53 > client53_Out.txt
if diff client53_Out.txt clientH1_GT.txt > /dev/null; then
        echo "Client53 output matches"
else
        echo "CLIENT53 OUTPUT DIFFER"
        diff client53_Out.txt clientH1_GT.txt
fi

#CLIENT 54 TEST
./client54 > client54_Out.txt
if diff client54_Out.txt clientH1_GT.txt > /dev/null; then
        echo "Client54 output matches"
else
        echo "CLIENT54 OUTPUT DIFFER"
        diff client54_Out.txt clientH1_GT.txt
fi

#CLIENT 55 TEST
./client55 > client55_Out.txt
if diff client55_Out.txt clientH1_GT.txt > /dev/null; then
        echo "Client55 output matches"
else
        echo "CLIENT55 OUTPUT DIFFER"
        diff client55_Out.txt clientH1_GT.txt
fi

#CLIENT 56 TEST
./client56 > client56_Out.txt
if diff client56_Out.txt clientH1_GT.txt > /dev/null; then
        echo "Client56 output matches"
else
        echo "CLIENT56 OUTPUT DIFFER"
        diff client56_Out.txt clientH1_GT.txt
fi

#CLIENT 57 TEST
./client57 > client57_Out.txt
if diff client57_Out.txt clientH1_GT.txt > /dev/null; then
        echo "Client57 output matches"
else
        echo "CLIENT57 OUTPUT DIFFER"
        diff client57_Out.txt clientH1_GT.txt
fi

#CLIENT 58 TEST
./client58 > client58_Out.txt
if diff client58_Out.txt clientC1_GT.txt > /dev/null; then
        echo "Client58 output matches"
else
        echo "CLIENT58 OUTPUT DIFFER"
        diff client58_Out.txt clientC1_GT.txt
fi

#CLIENT 59 TEST
./client59 > client59_Out.txt
if diff client59_Out.txt clientC1_GT.txt > /dev/null; then
        echo "Client59 output matches"
else
        echo "CLIENT59 OUTPUT DIFFER"
        diff client59_Out.txt clientC1_GT.txt
fi

#CLIENT 60 TEST
./client60 > client60_Out.txt
if diff client60_Out.txt clientH2_GT.txt > /dev/null; then
        echo "Client60 output matches"
else
        echo "CLIENT60 OUTPUT DIFFER"
        diff client60_Out.txt clientH2_GT.txt
fi

#CLIENT 61 TEST
./client61 > client61_Out.txt
if diff client61_Out.txt clientH2_GT.txt > /dev/null; then
        echo "Client61 output matches"
else
        echo "CLIENT61 OUTPUT DIFFER"
        diff client61_Out.txt clientH2_GT.txt
fi

#CLIENT 62 TEST
./client62 > client62_Out.txt
if diff client62_Out.txt clientE1_GT.txt > /dev/null; then
        echo "Client62 output matches"
else
        echo "CLIENT62 OUTPUT DIFFER"
        diff client62_Out.txt clientE1_GT.txt
fi

#CLIENT 63 TEST
./client63 > client63_Out.txt
if diff client63_Out.txt clientE1_GT.txt > /dev/null; then
        echo "Client63 output matches"
else
        echo "CLIENT63 OUTPUT DIFFER"
        diff client63_Out.txt clientE1_GT.txt
fi


#CLIENT 38/64 TEST
./client38 > client38_Out.txt & 
client38PID=$!
sleep 1
./client64 > client64_Out.txt &
client64PID=$!

# Wait for both clients to finish
wait $client38PID
wait $client64PID

# Check output for client49
if diff client64_Out.txt clientC2_GT.txt > /dev/null; then
        echo "Client64 output matches"
else
        echo "CLIENT64 OUTPUT DIFFER"
        diff client64_Out.txt clientC2_GT.txt
fi


#CLIENT 38/65 TEST
./client38 > client38_Out.txt & 
client38PID=$!
sleep 1
./client65 > client65_Out.txt &
client65PID=$!

# Wait for both clients to finish
wait $client38PID
wait $client65PID

# Check output for client65
if diff client65_Out.txt clientC2_GT.txt > /dev/null; then
        echo "Client65 output matches"
else
        echo "CLIENT65 OUTPUT DIFFER"
        diff client65_Out.txt clientC2_GT.txt
fi


#CLIENT 38/66 TEST
./client38 > client38_Out.txt & 
client38PID=$!
sleep 1
./client66 > client66_Out.txt &
client66PID=$!

# Wait for both clients to finish
wait $client38PID
wait $client66PID

# Check output for client66
if diff client66_Out.txt clientC2_GT.txt > /dev/null; then
        echo "Client66 output matches"
else
        echo "CLIENT66 OUTPUT DIFFER"
        diff client66_Out.txt clientC2_GT.txt
fi


#CLIENT 67 TEST
./client67 > client67_Out.txt
if diff client67_Out.txt clientH1_GT.txt > /dev/null; then
        echo "Client67 output matches"
else
        echo "CLIENT67 OUTPUT DIFFER"
        diff client67_Out.txt clientH1_GT.txt
fi


#CLIENT 68 TEST
./client68 > client68_Out.txt
if diff client68_Out.txt clientH1_GT.txt > /dev/null; then
        echo "Client68 output matches"
else
        echo "CLIENT68 OUTPUT DIFFER"
        diff client68_Out.txt clientH1_GT.txt
fi


#CLIENT 69/70/71/72 TEST
# many different clients connect to the server and some dissapear
./client69 > client69_Out.txt & 
client69PID=$!
sleep 1
./client70 > client70_Out.txt &
client70PID=$!
sleep 1
./client71 > client71_Out.txt &
client71PID=$!
sleep 1
./client72 > client72_Out.txt &
client72PID=$!

# Wait for clients to finish
wait $client69PID
wait $client70PID
wait $client71PID
wait $client72PID

# Check output for client69
if diff client69_Out.txt client69_GT.txt > /dev/null; then
    echo "Client69 output matches"
else
    echo "CLIENT69 OUTPUT DIFFER"
    diff client69_Out.txt client69_GT.txt
fi

# Check output for client70
if diff client70_Out.txt client70_GT.txt > /dev/null; then
    echo "Client70 output matches"
else
    echo "CLIENT70 OUTPUT DIFFER"
    diff client70_Out.txt client70_GT.txt
fi

# Check output for client71
if diff client71_Out.txt client71_GT.txt > /dev/null; then
    echo "Client71 output matches"
else
    echo "CLIENT71 OUTPUT DIFFER"
    diff client71_Out.txt client71_GT.txt
fi

# Check output for client72
if diff client72_Out.txt client72_GT.txt > /dev/null; then
    echo "Client72 output matches"
else
    echo "CLIENT72 OUTPUT DIFFER"
    diff client72_Out.txt client72_GT.txt
fi


#CLIENT 73 TEST
# writes until the limit of number of clients is reached
./client73 > client73_Out.txt
if diff client73_Out.txt client73_GT.txt > /dev/null; then
    echo "Client73 output matches"
else
    echo "CLIENT73 OUTPUT DIFFER"
    diff client73_Out.txt client73_GT.txt
fi

#CLIENT 74 TEST
# client writes partial HELLO and combined HELLO / EXIT
./client74 > client74_Out.txt
if diff client74_Out.txt client74_GT.txt > /dev/null; then
        echo "Client74 output matches"
else
        echo "CLIENT74 OUTPUT DIFFER"
        diff client74_Out.txt client74_GT.txt
fi



#CLIENT 75/76 TEST
# write CHAT message header split up with sleep in between
# write CHAT message body split up with sleep in between
./client75 > client75_Out.txt & 
client75PID=$!
sleep 1
./client76 > client76_Out.txt &
client76PID=$!

# Wait for both clients to finish
wait $client75PID
wait $client76PID

# Check output for client75
if diff client75_Out.txt client75_GT.txt > /dev/null; then
    echo "Client75 output matches"
else
    echo "CLIENT75 OUTPUT DIFFER"
    diff client75_Out.txt client75_GT.txt
fi

# Check output for client76
if diff client76_Out.txt client76_GT.txt > /dev/null; then
    echo "Client76 output matches"
else
    echo "CLIENT76 OUTPUT DIFFER"
    diff client76_Out.txt client76_GT.txt
fi


#CLIENT 38/77 TEST
./client38 > client38_Out.txt & 
client38PID=$!
sleep 1
./client77 > client77_Out.txt &
client77PID=$!

# Wait for both clients to finish
wait $client38PID
wait $client77PID

# Check output for client77
if diff client77_Out.txt clientC2_GT.txt > /dev/null; then
        echo "Client77 output matches"
else
        echo "CLIENT77 OUTPUT DIFFER"
        diff client77_Out.txt clientC2_GT.txt
fi


#CLIENT 78 TEST
# client writes combined HELLO part CHAT message and times out
./client78 > client78_Out.txt
if diff client78_Out.txt client78_GT.txt > /dev/null; then
        echo "Client78 output matches"
else
        echo "CLIENT78 OUTPUT DIFFER"
        diff client78_Out.txt client78_GT.txt
fi



# #CLIENT Y TEST
# ./clientY > clientY_Out.txt
# if diff clientY_Out.txt clientY_GT.txt > /dev/null; then
#         echo "ClientY output matches"
# else
#         echo "CLIENTY OUTPUT DIFFER"
#         diff clientY_Out.txt clientY_GT.txt
# fi



kill $serverPID