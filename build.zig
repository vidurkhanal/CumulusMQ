const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});

    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "CumulusMQ",
        .target = target,
        .optimize = optimize,
    });

    const flags = [_][]const u8{
        "-std=c++17",
        "-pedantic",
        "-Wall",
        "-W",
        "-Wno-missing-field-initializers",
    };

    // TODO: Can introduce function to build this array {take a look at bun's repo} instead of manually writing all the files. But examples from ZigFoundation like Raylib and 7Zip do this manually. So Kept it in this format.
    const source_files = [_][]const u8{ "src/main.cpp", "src/memory.cpp", "src/storage.cpp", "src/conn.cpp", "src/ioutils.cpp", "src/server.cpp", "src/topic.cpp", "src/utils.cpp", "src/env.cpp" };

    exe.addCSourceFiles(.{ .files = &source_files, .flags = &flags });
    exe.linkLibCpp();

    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);

    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
