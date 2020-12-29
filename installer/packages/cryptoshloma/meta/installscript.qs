function Component()
{
    // default constructor
}

Component.prototype.createOperations = function()
{
    // call default implementation to actually install README.txt!
    component.createOperations();

    if (systemInfo.productType === "windows") {
        component.addOperation("CreateShortcut", "@TargetDir@/Launcher.exe", "@StartMenuDir@/CryptoShloma.lnk",
            "workingDirectory=@TargetDir@", "iconPath=@TargetDir@/Launcher.exe",
            "iconId=0", "description=CryptoShloma");
        component.addOperation("CreateShortcut", 
                        "@TargetDir@/Launcher.exe",// target
                        "@DesktopDir@/CryptoShloma.lnk",// link-path
                        "workingDirectory=@TargetDir@",// working-dir
                        "iconPath=@TargetDir@/Launcher.exe", "iconId=0",// icon
                        "description=CryptoShloma");// description
    }
}

