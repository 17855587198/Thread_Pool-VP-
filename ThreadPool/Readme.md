ThreadPool pool #定义线程池
pool.setMode(fixed(default)|cached);
pool.start();

Result result=pool.submitTask(concreteTask)