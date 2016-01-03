#import <Foundation/Foundation.h>
#import <Hopper/Hopper.h>
#import "DYLD.hpp"

@interface DYLDSharedCacheLoader : NSObject<FileLoader>

@end

@implementation DYLDSharedCacheLoader {
  NSObject<HPHopperServices> *_services;
}

#pragma mark - HopperPlugin

- (instancetype)initWithHopperServices:(NSObject <HPHopperServices> *)services {
  self = [super init];
  if (self) {
    _services = services;
  }
  return self;
}

- (HopperUUID *)pluginUUID {
  return [_services UUIDWithString:@"F5DF5BDE-99A6-477E-B5E7-FA20A8B63E8C"];
}

- (HopperPluginType)pluginType {
  return Plugin_Loader;
}

- (NSString *)pluginName {
  return @"DYLDSharedCacheLoader";
}

- (NSString *)pluginDescription {
  return @"DYLD Shared Cache Loader";
}

- (NSString *)pluginAuthor {
  return @"Ievgen Solodovnykov";
}

- (NSString *)pluginCopyright {
  return @"©2015 – Ievgen Solodovnykov";
}

- (NSString *)pluginVersion {
  return @"1.0.0";
}

#pragma mark - FileLoader

- (BOOL)canLoadDebugFiles {
  return NO;
}

- (NSArray *)detectedTypesForData:(NSData *)data {
  DYLD::SharedCache cache([data bytes], [data length]);
  if (cache.load()) {
    NSObject<HPDetectedFileType> *type = [_services detectedType];
    type.fileDescription = @"DYLD Shared Cache (with fixed __LINKEDIT)";
    type.shortDescriptionString = @"dyld_shared_cache";
    type.compositeFile = YES;
    
    NSMutableDictionary *pathsMap = [NSMutableDictionary dictionary];
    NSMutableArray *optionsList = [NSMutableArray array];
    auto paths = cache.getPaths();
    for (auto &p : paths) {
      NSString *path = @(p.c_str());
      NSString *name = [path lastPathComponent];
      NSString *option = [NSString stringWithFormat:@"%@ (%@)", name, path];
      pathsMap[option] = path;
      [optionsList addObject:option];
    }
    NSObject<HPLoaderOptionComponents> *options;
    options = [_services stringListComponentWithLabel:@"File" andList:optionsList];
    type.additionalParameters = @[options];
    type.internalObject = pathsMap;
    
    return @[type];
  }
  return @[];
}

- (FileLoaderLoadingStatus)loadData:(__unused NSData *)data usingDetectedFileType:(__unused DetectedFileType *)fileType options:(__unused FileLoaderOptions)options forFile:(__unused NSObject <HPDisassembledFile> *)file usingCallback:(__unused FileLoadingCallbackInfo)callback {
  return DIS_NotSupported;
}

- (FileLoaderLoadingStatus)loadDebugData:(__unused NSData *)data forFile:(__unused NSObject <HPDisassembledFile> *)file usingCallback:(__unused FileLoadingCallbackInfo)callback {
  return DIS_NotSupported;
}

- (void)fixupRebasedFile:(__unused NSObject <HPDisassembledFile> *)file withSlide:(__unused int64_t)slide originalFileData:(__unused NSData *)fileData {}

- (NSData *)extractFromData:(NSData *)data usingDetectedFileType:(NSObject <HPDetectedFileType> *)fileType returnAdjustOffset:(__unused uint64_t *)adjustOffset {
  DYLD::SharedCache cache([data bytes], [data length]);
  if (cache.load()) {
    NSUInteger index = [(NSObject<HPLoaderOptionComponents> *)(fileType.additionalParameters)[0] selectedStringIndex];
    NSString *name = [(NSObject<HPLoaderOptionComponents> *)(fileType.additionalParameters)[0] stringList][index];
    NSString *path = fileType.internalObject[name];
    auto library = cache.getLibrary([path UTF8String]);
    return [[NSData alloc] initWithBytes:library.data.data() length:library.data.size()];
  }
  return nil;
}

@end
