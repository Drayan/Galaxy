using UnrealBuildTool;
using System.IO;

public class GalaxyShaders : ModuleRules
{
    public GalaxyShaders(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "Renderer", "RenderCore", "RHI" });

        string EnginePath = Path.GetFullPath(Target.RelativeEnginePath);
        PublicIncludePaths.Add(EnginePath + "Source/Runtime/Renderer/Private");
    }
}