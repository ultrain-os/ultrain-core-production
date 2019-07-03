#monitor Uranus Node info
curl  --request POST --url http://127.0.0.1:8888/v1/monitor/monitor_node

#monitor ProposeMsg in UranusController::m_proposerMsgMap bt block id
curl  --request POST --url http://127.0.0.1:8888/v1/monitor/monitor_propose_msg --data ‘{"block_id":"12345678"}'

#monitor echo_message_info in UranusController::m_echoMsgMap by block id
curl  --request POST --url http://127.0.0.1:8888/v1/monitor/monitor_echo_msg --data '{"block_id":"12345678"}'

#monitor ProposeMsg vector in UranusController::m_cacheProposeMsgMap by RoundInfo
curl  --request POST --url http://127.0.0.1:8888/v1/monitor/monitor_propose_cache --data '{"blockNum":"1", “phase":"0"}'

#monitor EchoMsg vector in UranusController::m_cacheEchoMsgMap by RoundInfo
curl  --request POST --url http://127.0.0.1:8888/v1/monitor/monitor_echo_cache --data '{"blockNum":"1", “phase":"0"}'

#monitor all <block id, echo_message_info> pairs in UranusController::m_echoMsgAllPhase by RoundInfo
curl  --request POST --url http://127.0.0.1:8888/v1/monitor/monitor_echo_ap_cache --data '{"blockNum":"1", "phase":"0"}'
