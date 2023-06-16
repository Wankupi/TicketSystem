<script setup lang="ts">

import { reactive, ref, provide } from 'vue';
import { backend_key, current_user_key, PageState, pageState_key, bill_info_key, type Bill } from "@/keys";
import addUser from '@/pages/add_user.vue';
import queryUser from '@/pages/query_user.vue';
import Login from '@/pages/login.vue';
import ShowCurrentUser from "@/components/user_status.vue";
import addTrain from "@/pages/add_train.vue";
import queryTrain from '@/pages/query_train.vue';
import queryTicket from '@/pages/query_ticket.vue'
import Buy_ticket from '@/pages/buy_ticket.vue';
import Query_order from '@/pages/query_order.vue';

let pageState = ref(PageState.login);
provide(pageState_key, pageState);

let connection_status = ref(0);
let backend = reactive(new WebSocket("ws://121.36.39.132:6001"));
provide(backend_key, backend);

let current_user = ref("");
provide(current_user_key, current_user);

let bill: Bill = reactive({ from: '', leave: '', seat: 0, to: '', trainID: '' });
provide(bill_info_key, bill);

backend.addEventListener('open', () => { connection_status.value = 1; });
backend.addEventListener('close', () => { connection_status.value = 2; });
backend.addEventListener('error', () => { connection_status.value = 3; });

let result = ref("");
backend.addEventListener("message", (msg) => { result.value = msg.data; })

</script>

<template>
  <el-container>
    <el-aside width="20em">
      <el-scrollbar>
        <h2>选项</h2>
        <div></div>
        <el-menu>
          <!-- <el-menu-item @click="pageState = PageState.login" :disabled="current_user">登陆</el-menu-item> -->
          <el-menu-item @click="pageState = PageState.query_profile" :disabled="!current_user">用户信息</el-menu-item>
          <el-menu-item @click="pageState = PageState.query_train">列车信息</el-menu-item>
          <el-menu-item @click="pageState = PageState.query_ticket">车票查询</el-menu-item>
          <el-menu-item @click="pageState = PageState.buy_ticket" :disabled="!current_user">购票</el-menu-item>
          <el-menu-item @click="pageState = PageState.query_order" :disabled="!current_user">订单</el-menu-item>
          <el-menu-item @click="pageState = PageState.add_user">添加用户</el-menu-item>
          <el-menu-item @click="pageState = PageState.add_train">添加列车</el-menu-item>
        </el-menu>
      </el-scrollbar>
    </el-aside>
    <el-container>
      <el-header :class="{ flex: true, stat_ok: connection_status == 1, st_wrong: connection_status != 1 }">
        <h1>Train Ticket System</h1>
        <ShowCurrentUser></ShowCurrentUser>
      </el-header>
      <el-main>
        <div v-show="pageState == PageState.login">
          <Login></Login>
        </div>
        <div v-show="pageState == PageState.add_user">
          <addUser></addUser>
        </div>
        <div v-show="pageState == PageState.query_profile">
          <queryUser></queryUser>
        </div>
        <div v-show="pageState == PageState.add_train">
          <addTrain></addTrain>
        </div>
        <div v-show="pageState == PageState.query_train">
          <queryTrain></queryTrain>
        </div>
        <div v-show="pageState == PageState.query_ticket">
          <queryTicket></queryTicket>
        </div>
        <div v-show="pageState == PageState.buy_ticket">
          <Buy_ticket></Buy_ticket>
        </div>
        <div v-show="pageState == PageState.query_order">
          <Query_order></Query_order>
        </div>
      </el-main>
    </el-container>
  </el-container>
</template>

<style scoped>
.stat_ok {
  background-color: skyblue;
}

.st_wrong {
  background-color: red;
}
</style>