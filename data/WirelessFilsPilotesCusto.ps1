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
        <h2 class="content-subhead">Fils Pilotes<span id="l3"><h6 style="display:inline"><b> Loading...</b></h6></span></h2>
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
            $.getJSON("/gc1",function(GC1){
                $.each(GC1,function(k,v){
                    if(k.match(/fp[0-9]n/)) $('#'+k).text(v);
                })
            });
            $("#l3").fadeOut();
        })
        .fail(function(){
            $("#l3").html('<h4 style="display:inline;color:red;"><b> Failed</b></h4>');
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
        
        <h2 class="content-subhead">Fils Pilotes<span id="l1"><h6 style="display:inline"><b> Loading...</b></h6></span></h2>
        <form class="pure-form pure-form-aligned" id='f1'>
            <fieldset>

        <div id='fpnl'></div>

                <div class="pure-controls">
                    <input type='submit' value='Save' class="pure-button pure-button-primary" disabled>
                </div>
            </fieldset>
        </form>
        <span id='r1'></span>
'@
        ;
        HTMLScript=@'
        $("#f1").submit(function(event){
            $("#r1").html("Saving Configuration...");
            $.post("/sc1",$("#f1").serialize(),function(){ 
                $("#f1").hide();
                var reload5sec=document.createElement('script');
                reload5sec.text='var count=4;var cdi=setInterval(function(){$("#cd").text(count);if(!count){clearInterval(cdi);location.reload();}count--;},1000);';
                $('#r1').html('<h3><b>Configuration saved <span style="color: green;">successfully</span>. System is restarting now.</b></h3>This page will be reloaded in <span id="cd">5</span>sec.').append(reload5sec);
            }).fail(function(){
                $('#r1').html('<h3><b>Configuration <span style="color: red;">error</span>.</b></h3>');
            });
            event.preventDefault();
        });
'@
        ;
        HTMLScriptInReady=@'
        $("#fpnl").innerHTML='';
        $.getJSON("/gc1", function(GC1){
            $.each(GC1,function(k,v){
                if(k.match(/fp[0-9]n/)){
                    var i=k[2];
                    $("#fpnl").append('<div class="pure-control-group"><label for="fp'+i+'n">FP'+i+' Name</label><input id="fp'+i+'n" name="fp'+i+'n" type="text"></div>');

                    if($('#'+k).prop('type')!='checkbox') $('#'+k).val(v);
                    else $('#'+k).prop("checked",v);
    
                    $('#'+k).trigger("change");
                }
            })

            $("input[type=submit]",$("#f1")).prop("disabled",false);
            $("#l1").fadeOut();
        })
        .fail(function(){
            $("#l1").html('<h6 style="display:inline;color:red;"><b> Failed</b></h6>');
        });
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