#Script to prepare Web files
# - Integrate customization of the application
# - GZip of resultant web files
# - and finally convert compressed web files to C++ header in PROGMEM

#List here web files specific to this project/application
$specificFiles="",""

#list here files that need to be merged with Common templates
$applicationName="WirelessFilsPilotes"
$shortApplicationName="WFP1/4/8"
$templatesWithCustoFiles=@{
    #---Status.html---
    "status.html"=@{
        ApplicationName=$applicationName
        ;
        ShortApplicationName=$shortApplicationName
        ;
        HTMLContent=@'
        <h2 class="content-subhead">FilsPilotes <span id="l2"><h6 style="display:inline"><b> Loading...</b></h6></span></h2>
        <span id="FPList"></span>
'@
        ;
        HTMLScriptInReady=@'
        function generateFPButtons(fpNumber,currentValue){
            var ret='<div class="pure-button-group" role="group">';
            ret+='<style scoped>.pure-button-active{background: rgb(31, 141, 214);}</style>';
            ret+='<button id="FP'+fpNumber+'v51" class="pure-button'+(currentValue>50?' pure-button-active':'')+'">Confort</button>';
            ret+='<button id="FP'+fpNumber+'v41" class="pure-button'+(currentValue>40&&currentValue<51?' pure-button-active':'')+'">Confort-1</button>';
            ret+='<button id="FP'+fpNumber+'v31" class="pure-button'+(currentValue>30&&currentValue<41?' pure-button-active':'')+'">Confort-2</button>';
            ret+='<button id="FP'+fpNumber+'v21" class="pure-button'+(currentValue>20&&currentValue<31?' pure-button-active':'')+'">Eco</button>';
            ret+='<button id="FP'+fpNumber+'v11" class="pure-button'+(currentValue>10&&currentValue<21?' pure-button-active':'')+'">Hors Gel</button>';
            ret+='<button id="FP'+fpNumber+'v1" class="pure-button'+(currentValue<11?' pure-button-active':'')+'">Arrêt</button>';
            ret+='</div>';

            return ret;
        }

        function showCurrentFP(fpNumber,newValue){
            if(newValue<51) $("#FP"+fpNumber+"v51").removeClass("pure-button-active"); else $("#FP"+fpNumber+"v51").addClass("pure-button-active");
            if(newValue<41 || newValue>50) $("#FP"+fpNumber+"v41").removeClass("pure-button-active"); else $("#FP"+fpNumber+"v41").addClass("pure-button-active");
            if(newValue<31 || newValue>40) $("#FP"+fpNumber+"v31").removeClass("pure-button-active"); else $("#FP"+fpNumber+"v31").addClass("pure-button-active");
            if(newValue<21 || newValue>30) $("#FP"+fpNumber+"v21").removeClass("pure-button-active"); else $("#FP"+fpNumber+"v21").addClass("pure-button-active");
            if(newValue<11 || newValue>20) $("#FP"+fpNumber+"v11").removeClass("pure-button-active"); else $("#FP"+fpNumber+"v11").addClass("pure-button-active");
            if(newValue>10) $("#FP"+fpNumber+"v1").removeClass("pure-button-active"); else $("#FP"+fpNumber+"v1").addClass("pure-button-active");
        }

        function assignButtonsActions(fpNumber){

            $("#FP"+fpNumber+"v51").click(function(){$.get("/setFP?FP"+fpNumber+"=51",function(){showCurrentFP(fpNumber,51);})});
            $("#FP"+fpNumber+"v41").click(function(){$.get("/setFP?FP"+fpNumber+"=41",function(){showCurrentFP(fpNumber,41);})});
            $("#FP"+fpNumber+"v31").click(function(){$.get("/setFP?FP"+fpNumber+"=31",function(){showCurrentFP(fpNumber,31);})});
            $("#FP"+fpNumber+"v21").click(function(){$.get("/setFP?FP"+fpNumber+"=21",function(){showCurrentFP(fpNumber,21);})});
            $("#FP"+fpNumber+"v11").click(function(){$.get("/setFP?FP"+fpNumber+"=11",function(){showCurrentFP(fpNumber,11);})});
            $("#FP"+fpNumber+"v1").click(function(){$.get("/setFP?FP"+fpNumber+"=1",function(){showCurrentFP(fpNumber,1);})});
        }

        $.getJSON("/gs1", function(GS1){

            $.each(GS1,function(k,v){
                $('#FPList').append('FP'+k[2]+' : <b><span id="fp'+k[2]+'n"></span></b>'+ generateFPButtons(k[2],v) +'<br>');
                assignButtonsActions(k[2]);
            })
            $("#l2").fadeOut();
        })
        .fail(function(){
            $("#l2").html('<h4 style="display:inline;color:red;"><b> Failed</b></h4>');
        });

        $.getJSON("/gc",function(GC){
            $.each(GC,function(k,v){
                if(k.match(/fp[0-9]n/)) $('#'+k).text(v);
            })
        });
'@
    }
    ;
    #---config.html---
    "config.html"=@{
        ApplicationName=$applicationName
        ;
        ShortApplicationName=$shortApplicationName
        ;
        HTMLContent=@'
        <h2 class="content-subhead">WFP</h2>
        <div id='fpnl'></div>
'@
        ;
        HTMLScript=@'
'@
        ;
        HTMLFillinConfigForm=@'
        $("#fpnl").innerHTML='';
        $.each(GC,function(k,v){
            if(k.match(/fp[0-9]n/)){
                var i=k[2];
                $("#fpnl").append('<div class="pure-control-group"><label for="fp'+i+'n">FP'+i+' Name</label><input id="fp'+i+'n" name="fp'+i+'n" type="text"></div>');
                $('#'+k).val(v);
            }
        })
'@
    }
    ;
    #---fw.html---
    "fw.html"=@{
        ApplicationName=$applicationName
        ;
        ShortApplicationName=$shortApplicationName
    }
    ;
    #---discover.html---
    "discover.html"=@{
        ApplicationName=$applicationName
        ;
        ShortApplicationName=$shortApplicationName
    }
}

#call script that prepare Common Web Files and contain compression/Convert/Merge functions
. ..\src\data\_prepareCommonWebFiles.ps1

$path=(Split-Path -Path $MyInvocation.MyCommand.Path)
$templatePath=($path+"\..\src\data")

Write-Host "--- Prepare Application Web Files ---"
Convert-TemplatesWithCustoToCppHeader -templatePath $templatePath -filesAndCusto $templatesWithCustoFiles -destinationPath $path
Convert-FilesToCppHeader -Path $path -FileNames $specificFiles
Write-Host ""