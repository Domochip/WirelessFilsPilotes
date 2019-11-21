//prepare equipment

$topicArray=array('WFP8/1','WFP8/2','WFP8/3','WFP8/4','WFP8/5','WFP8/6','WFP8/7','WFP8/8');
$orderDashboardWidget='FilPilote6Ordres';
$orderMobileWidget='FilPilote6Ordres';


function createSetButtonCmd($eqLogicFP,$orderCmdId,$name,$displayOrder,$orderValue,$icon)
{
    global $scenario;

    try{
        $buttonCmd = cmd::byString('#'.$eqLogicFP->getHumanName().'['.$name.']#');
        $scenario->setLog($name.' Found');
    }
    catch(Exception $e){
        $buttonCmd=new MQTTCmd();
        $buttonCmd->setName($name);
        $buttonCmd->setEqLogic_id($eqLogicFP->getId());
        $buttonCmd->setEqType('MQTT')->setType('action')->setSubType('other');
        $scenario->setLog($name.' Created');
    }

    $buttonCmd->setDisplay('icon','<i class="'.$icon.'"></i>');
    $buttonCmd->setOrder($displayOrder);
    $buttonCmd->setValue($orderCmdId);
    $buttonCmd->setConfiguration('topic',$eqLogicFP->getConfiguration('topic').'/command');
    $buttonCmd->setConfiguration('request',$orderValue);
    $buttonCmd->setConfiguration('retain','0');
    $buttonCmd->save();

    $scenario->setLog($name.' OK');
}

function configureFPEquipment($topicFP,$orderDashboardWidget,$orderMobileWidget){

    global $scenario;

    //Find Equipment
    $eqFP=eqLogic::byTypeAndSearhConfiguration('MQTT','%"topic":"'.$topicFP.'"%')[0];
    $eqFP->setIsEnable(1)->setIsVisible(1);
    $eqFP->setCategory('heating',1);
    $eqFP->setConfiguration('icone','chauffage');
    $eqFP->save();
    $scenario->setLog('Equipment OK');

    //Configure "order" command
    $orderCmd=cmd::byEqLogicIdCmdName($eqFP->getId(),"order");
    $orderCmd->setSubType('numeric');
    $orderCmd->setIsHistorized(1);
    $orderCmd->setConfiguration('historizeMode','none');
    $orderCmd->setConfiguration('historyPurge','-7 days');
    $orderCmd->setOrder(0);
    $orderCmd->setDisplay('showNameOndashboard','0');
    $orderCmd->setTemplate('dashboard',$orderDashboardWidget);
    $orderCmd->setTemplate('mobile',$orderMobileWidget);
    $orderCmd->save();

    // Create/Configure buttons to send command
    createSetButtonCmd($eqFP,$orderCmd->getId(),'Confort',1,'51','icon jeedom-thermo-chaud');
    createSetButtonCmd($eqFP,$orderCmd->getId(),'Confort-1',2,'41','icon jeedom-thermo-moyen');
    createSetButtonCmd($eqFP,$orderCmd->getId(),'Confort-2',3,'31','icon jeedom-thermo-froid');
    createSetButtonCmd($eqFP,$orderCmd->getId(),'Eco',4,'21','icon divers-triangular42');
    createSetButtonCmd($eqFP,$orderCmd->getId(),'Hors Gel',5,'11','icon nature-snowflake');
    createSetButtonCmd($eqFP,$orderCmd->getId(),'Arrêt',6,'1','fa fa-power-off');

    //
    try{
        $commandCmd = cmd::byString('#'.$eqFP->getHumanName().'[command]#');
        $scenario->setLog('"command" Found');
    }
    catch(Exception $e){
        $commandCmd=new MQTTCmd();
        $commandCmd->setName('command');
        $commandCmd->setEqLogic_id($eqFP->getId());
        $commandCmd->setEqType('MQTT')->setType('info')->setSubType('numeric');
        $scenario->setLog('"command" Created');
    }

    $commandCmd->setOrder(7);
    $commandCmd->setIsVisible(0);
    $commandCmd->setConfiguration('topic',$eqFP->getConfiguration('topic').'/command');
    $commandCmd->save();

    $scenario->setLog('"command" OK');
}

foreach($topicArray as $topic){
    $scenario->setLog('=== Looking for topic "'.$topic.'" ===');
    configureFPEquipment(str_replace('/','\\\\/',$topic),$orderDashboardWidget,$orderMobileWidget);
    $scenario->setLog('=== "'.$topic.'" finished ===');
}