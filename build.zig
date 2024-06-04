const std = @import("std");

pub fn build(b: *std.Build) void {

    // Zig Compiler Target Options, all passed by user with this mechanism
    // e.g: zig build -Dtarget=x86_64-windows {...other flags}
    const target = b.standardTargetOptions(.{});

    // Zig Compiler Optimization Options, all passed by user with this mechanism
    // e.g: zig build -Doptimize=ReleaseSmall | ReleaseFast | ReleaseSafe | Debug {...other flags}
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "CumulusMQ",
        .target = target,
        .optimize = optimize,
    });

    // Flags of how you want to build you c++ proj
    const flags = [_][]const u8{
        "-std=c++17",
        "-pedantic",
        "-Wall",
        "-W",
        "-Wno-missing-field-initializers",
    };

    // All your CPP files
    // TODO: Can introduce function to build this array {take a look at bun's repo} instead of manually writting all the files. But adopters like Raylib and 7Zip do this manually. So Kept it in this format.
    const source_files = [_][]const u8{ "src/main.cpp", "src/memory.cpp", "src/storage.cpp", "src/conn.cpp", "src/ioutils.cpp", "src/server.cpp", "src/topic.cpp" };

    exe.addCSourceFiles(.{ .files = &source_files, .flags = &flags });
    exe.linkLibCpp();

    b.installArtifact(exe);

    // SETUP zig run build
    // This *creates* a Run step in the build graph, to be executed when another
    // step is evaluated that depends on it. The next line below will establish
    // such a dependency.
    const run_cmd = b.addRunArtifact(exe);

    // Don't use exec from zig-cache instead installArtifact and run that.
    run_cmd.step.dependOn(b.getInstallStep());

    // This allows the user to pass arguments to the application in the build
    // command itself, like this: `zig build run -- arg1 arg2 etc`
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
